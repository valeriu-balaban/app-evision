// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -lpthread -shared-libgcc opencv.cpp -o opencv

#include "evision.h"
#include "tracer.h"
#include "gpio.h"


// Global variables
GPIO pwm_right(6, "out"), pwm_left(7, "out");
int high_right = 0, high_left = 0, period = 20000; //PWM high time in us

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

int main(int argc, char** argv)
{
    pthread_t processing_thread, pwm_thread;
    long time_old;
    int fps_counter = 0;
    int gui_key;
    
    // GUI setup
	cv::namedWindow(main_window_name);
	
	// Setup window
	cv::namedWindow(settings_window_name);
	cv::createTrackbar("Show Step   ", settings_window_name, &settings_show_step, LAST_STEP-1);
	cv::createTrackbar("Contrast    ", settings_window_name, &settings_contrast, 1);
	cv::createTrackbar("Mean Blur   ", settings_window_name, &settings_blur, 1);
	cv::createTrackbar("Threshold   ", settings_window_name, &settings_threshold, 255);
	cv::createTrackbar("Servo Right ", settings_window_name, &high_right, 20000);
	cv::createTrackbar("Servo Left  ", settings_window_name, &high_left, 20000);
	
	pthread_create(&processing_thread, NULL, processing_thread_function, NULL);    
	pthread_create(&pwm_thread, NULL, pwm_thread_function, NULL);
	
	
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
			if(gui_key == 43){
				running = false;
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
		guiframe = frame;
		new_frame = 1;
	}
}

void *processing_thread_function(void* unsused)
{
	cv::VideoCapture 	cap(0); // camera interface    
    cv::Mat 			frame, cam_frame, bw_frame, blur_frame, contrast_frame;
    cv::Mat				threshold_frame, canny_frame, contour_frame;
    Tracer				processing_tracer;
    
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
		processing_tracer.event("Appling threshold");
		send_frame_to_gui(threshold_frame, THRESHOLD_IMAGE);
		
		// detect and paint contours
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(threshold_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		contour_frame = blur_frame;
		cv::drawContours(bw_frame, contours, -1, cv::Scalar(0), 2); 
		processing_tracer.event("Contour detection");
		send_frame_to_gui(contour_frame, CONTOUR_IMAGE);
	}
	
	processing_tracer.end();
	pthread_exit(NULL);
}

void *pwm_thread_function(void *unused){
	
	while(running){
		int h_r = high_right, h_l = high_left; //local var
		int delay1 = h_r > h_l ? h_l : h_r;
		int delay2 = h_r > h_l ? h_r-h_l : h_l-h_r;
		int delay3 = period - h_r - h_l;

		pwm_right.high();
		pwm_left.high();
		usleep(delay1);
		if(h_r > h_l)
			pwm_left.low();
		else 
			pwm_right.low();
		usleep(delay2);
		if(h_r > h_l)
			pwm_right.low();
		else
			pwm_left.low();
		usleep(delay3);
	}
	
	pthread_exit(NULL);
}

void add_info(int fps){
	// white canvas
	cv::Mat img_info (25, 320, CV_8UC3, cv::Scalar(255, 255, 255));

	cv::putText(img_info, std::string("FPS: ") + std::to_string(fps), cv::Point(5, 20), 
			cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0,0,255), 1, CV_AA);
			
	cv::imshow(settings_window_name, img_info);	
}
