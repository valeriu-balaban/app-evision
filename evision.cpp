// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -lpthread -shared-libgcc opencv.cpp -o opencv

#include "evision.h"
#include "tracer.h"
#include "gpio.h"


// Global variables
GPIO pwm_right(1, "out"), pwm_left(3, "out"); // led_right(2 , "out");
GPIO led_front(5, "out"), led_R(4, "out"), start(7 ,"out"); // 0,2,6 bulit
int high_right = 600, high_left = 600, period = 20000; //PWM high time in us
int top_edge = 50, servo_offset = 0, road_offset = 0;

// GUI globals
cv::Mat guiframe;
bool new_frame = false;
bool running = true;

// Window titles
char main_window_name[] = "Lightning Asystant :: Team eVision";
char settings_window_name[] = "Settings :: Team eVision";

// Settings variables
int settings_show_step = 0;
int settings_contrast = 0;
int settings_blur = 1;
int settings_threshold = 128;
int settings_servo_offset = 500;
int settings_road_approx = 5;
int settings_middle_line = 240;

// Graphics
std::vector<std::vector<cv::Point>> contours;

//Functions declaration
void pwm_servo_right(int);
void pwm_servo_left(int);
int car_position(int);
int obstacle_position(int);


int main(int argc, char** argv)
{
    pthread_t processing_thread;
    long time_old;
    int fps_counter = 0;
    int gui_key;
    
	// for cpu affinity
	cpu_set_t cpuset; 
	int cpu = 0;
	
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 2 on cpuset
	sched_setaffinity(0, sizeof(cpuset), &cpuset);
    
    // process priority
    setpriority(PRIO_PROCESS, 0, -20);
    
    // GUI setup
	cv::namedWindow(main_window_name);
	
	// Setup window
	cv::namedWindow(settings_window_name);
	cv::createTrackbar("Show Step   ", settings_window_name, &settings_show_step, LAST_STEP-1);
	cv::createTrackbar("Contrast    ", settings_window_name, &settings_contrast, 1);
	cv::createTrackbar("Mean Blur   ", settings_window_name, &settings_blur, 1);
	cv::createTrackbar("Threshold   ", settings_window_name, &settings_threshold, 255);
	cv::createTrackbar("Road Approx ", settings_window_name, &settings_road_approx, 30);
	cv::createTrackbar("Middle Line ", settings_window_name, &settings_middle_line, 320);
	cv::createTrackbar("Servo Offset", settings_window_name, &settings_servo_offset, 1000);
	cv::createTrackbar("Servo Right ", settings_window_name, &high_right, 1500);
	cv::createTrackbar("Servo Left  ", settings_window_name, &high_left, 1500);
	
	pthread_create(&processing_thread, NULL, processing_thread_function, NULL);    
	//pthread_create(&pwm_thread, NULL, pwm_thread_function, NULL);
	
	
    while(running){
    
    	while(!new_frame){ } // loop
    	
		cv::imshow(main_window_name, guiframe);
		
		// calculate fps
		if(time(NULL) != time_old){
			add_info(fps_counter);
			fps_counter = 1;
			time_old = time(NULL);
		} else {
			fps_counter++;
		}
		
		new_frame = 0;
		
		gui_key = cv::waitKey(5);
		if(gui_key >= 0) {
			gui_key %= 0xFF;
			
			// ESC
			if((gui_key == 43) || (gui_key == 27)){
				led_front.low();
				led_R.low();
				running = false;
			} else if((gui_key == 48) || (gui_key == 32)) {
				start.high(); // 1 = start/stop
				usleep(50000);
				start.low();
			} else if((gui_key == 50) || (gui_key == 66)) {
				led_front.toggle(); // 2 = faruri
			} else if((gui_key == 51) || (gui_key == 67)) {
				led_R.toggle(); // 3 = far pieton
			} else {
				std::cout << gui_key << std::endl;
			}
		} 
    }
	
	pthread_join(processing_thread, NULL);
	
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

void send_frame_to_gui(cv::Mat &frame, int step){
	if((new_frame == 0) && (step == settings_show_step)){
		//cv::pyrUp(frame, guiframe);			
		frame.copyTo(guiframe);
		new_frame = 1;
	}
}

bool contour_area(int a, int b){
	return cv::contourArea(contours[a]) > cv::contourArea(contours[b]);
}


bool get_obstacle(
	int parent, 
	std::vector<std::vector<cv::Point>> contours, 
	std::vector<cv::Vec4i> hierarchy,
	cv::Rect &obstacle){
	
	int start = hierarchy[parent][2];
	int closest = -1;
	bool found = false;
	
	while(start > 0){
		
		if(closest == -1){
			closest = start;
			found = true;
		} else if(cv::boundingRect(contours[start]).y > cv::boundingRect(contours[closest]).y){
			closest = start;
		}

		start = hierarchy[start][0];
	}
	
	if(found){
		obstacle = cv::boundingRect(contours[closest]);
	}
	
	return found;	
}

void draw_obstacles(cv::Mat &threshold_frame, cv::Mat &cam_frame){
	std::vector<std::vector<cv::Point>> road(2);
	std::vector<cv::Vec4i> hierarchy;
	
	cv::findContours(threshold_frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);		
	if(contours.size() > 1){
		
		// create index vector
		std::vector<int> contour_indexes(contours.size());
		
		for(unsigned int i = 0; i < contours.size(); i++){
			contour_indexes[i] = i;
		}				
			
		// sorting index vector acording to controur area
		std::sort(contour_indexes.begin(), contour_indexes.end(), contour_area);
		
		cv::approxPolyDP(cv::Mat(contours[contour_indexes[0]]), road[0], settings_road_approx, true);					
		cv::approxPolyDP(cv::Mat(contours[contour_indexes[1]]), road[1], settings_road_approx, true);					
		
		// validate road detection
		int road_edge = -1;
		for(unsigned int i = 0; i < road[0].size(); ++i){
			if((road[0][i].y > 235) && (road_edge == -1)){
				road_edge = road[0][i].x;
				 
			} else if((road[0][i].y > 235) && (road[0][i].x < road_edge)){
				
				road_edge = road[0][i].x;
			}				
		}
		
		if(road_edge < 0)
			return ;
		
		for(unsigned int i = 0; i < road[1].size(); ++i){
			if((road[1][i].y > 235) && (road[1][i].x > road_edge)){
				
				// wrong detection
				return;
			}				
		}
		
		// paint road area
		cv::drawContours(cam_frame, road, 0, cv::Scalar(0, 255 ,0), 2);	
		cv::drawContours(cam_frame, road, 1, cv::Scalar(255, 0, 0), 2);
		
		
		// get road offset
		// reference point is the left most point from the top edge of road contour
		int new_road_offset = 600;
		for(unsigned int i = 0; i < road[0].size(); ++i){
			if((road[0][i].y == top_edge) && (road[0][i].x < new_road_offset)){
				new_road_offset = road[0][i].x;
			}				
		}
		
		if((new_road_offset > 50) && (new_road_offset < 250)){
			road_offset = new_road_offset;
		}
		
		//std::cout << road_offset << std::endl;		
		
		cv::Rect obstacle;
		if(get_obstacle(contour_indexes[0], contours, hierarchy, obstacle)){
			// obtacle on the road: draw rectangle, adjust pwm for servo, blink led
			cv::rectangle(cam_frame, obstacle, cv::Scalar(0, 0, 255));
			pwm_servo_right(high_right + servo_offset + road_offset + obstacle_position(obstacle.y + (obstacle.height / 2)));

			led_R.toggle();
		} else {
			led_R.low();
			pwm_servo_right(high_right + servo_offset + road_offset);
		}
		
		if(get_obstacle(contour_indexes[1], contours, hierarchy, obstacle)){
			cv::rectangle(cam_frame, obstacle, cv::Scalar(255, 0, 255));
	   	
   			pwm_servo_left(high_left + servo_offset + road_offset + car_position(0));
		} else {
		
			pwm_servo_left(high_left + servo_offset + road_offset);
		}
		
		cv::line(cam_frame, cv::Point(settings_middle_line, 320), cv::Point(settings_middle_line, top_edge), cv::Scalar(0, 255, 255));
	}
}


void *processing_thread_function(void* unsused)
{
	cv::VideoCapture 	cap(0); // camera interface    
    cv::Mat 			frame, cam_frame, bw_frame, blur_frame, contrast_frame;
    cv::Mat				threshold_frame, canny_frame, contour_frame;
    Tracer				processing_tracer;
    
    
    // for cpu affinity
	cpu_set_t cpuset; 
	int cpu = 1;
	
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 2 on cpuset
	sched_setaffinity(0, sizeof(cpuset), &cpuset);
    
    if(!cap.isOpened())  // check if we succeeded
    {
        std::cout << "Could not open default video device" << std::endl;
        pthread_exit(NULL);
        
    } 
        
   	while(running) {
		if( !cap.read(frame) ){
			std::cout << "Camera was disconected";			
			break;
		}		
		
		cv::pyrDown(frame, cam_frame);
		send_frame_to_gui(cam_frame, SENSOR_IMAGE);
		processing_tracer.start();
		
		// All processing are done on gray image 
		cv::cvtColor(cam_frame, bw_frame, CV_BGR2GRAY);
		processing_tracer.event("Convert to Gray");
		send_frame_to_gui(bw_frame, GRAY_IMAGE);
		
		// Increase contrast by distributing color histogram to contain all values
		if(settings_contrast){
			cv::equalizeHist(bw_frame, contrast_frame);
		} else {
			contrast_frame = bw_frame;
		}
		processing_tracer.event("Equalize histogram");
		send_frame_to_gui(contrast_frame, CONTRAST_IMAGE);
		
		// Apply a special blur filter which preserves edges
		if(settings_blur){
			cv::medianBlur(contrast_frame, blur_frame, 7);
		} else {
			blur_frame = contrast_frame;
		}
		processing_tracer.event("Median blur");
		send_frame_to_gui(blur_frame, BLUR_IMAGE);
		
		// detect edges using histeresys
		cv::Canny(contrast_frame, canny_frame, 30, 100);
		processing_tracer.event("Edge detection");
		send_frame_to_gui(canny_frame, CANNY_IMAGE);
		
		
		// Apply threshhold
		threshold(blur_frame, threshold_frame, settings_threshold, 255, CV_THRESH_BINARY);
		// Disable image top from detection to remove false edges
		cv::rectangle(threshold_frame, cv::Rect(0, 0, 320, top_edge), cv::Scalar(0), CV_FILLED);
		processing_tracer.event("Appling threshold");
		send_frame_to_gui(threshold_frame, THRESHOLD_IMAGE);
		
		// detect and paint contours		
		draw_obstacles(threshold_frame, cam_frame);
		processing_tracer.event("Contour detection");
		send_frame_to_gui(cam_frame, CONTOUR_IMAGE);
	}
	
	processing_tracer.end();
	pthread_exit(NULL);
}

void pwm_servo_right(int h_r){
	int static local_hr = 0;
	if(abs(h_r - local_hr) > 2){
		local_hr = h_r;
		pwm_right.high();
		usleep(local_hr);
		pwm_right.low();
		usleep(period - local_hr);
	}
}

void pwm_servo_left(int h_l){
	int static local_hl = 0;
	if(abs(h_l - local_hl) > 2){
		local_hl = h_l;
		pwm_left.high();
		usleep(local_hl);
		pwm_left.low();
		usleep(period - local_hl);
	}
}

int obstacle_position(int y_position){
	if(y_position > top_edge){
		return int ( 0.9 * y_position + 15);
	} else {
		return 0;
	}
}

int car_position(int y_position){
	if(y_position > top_edge){
		return 1;
	} else {
		return 0;
	}
}

void add_info(int fps){
	// white canvas
	cv::Mat img_info (25, 320, CV_8UC3, cv::Scalar(255, 255, 255));

	cv::putText(img_info, std::string("FPS: ") + std::to_string(fps), cv::Point(5, 20), 
			cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0,0,255), 1, CV_AA);
			
	cv::imshow(settings_window_name, img_info);	
}
