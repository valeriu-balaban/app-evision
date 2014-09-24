// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -lpthread -shared-libgcc opencv.cpp -o opencv

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "gpio.h"
#include "tracer.h"

using namespace cv;
using namespace std;

enum Processing_Step{
	SENSOR_IMAGE,
	GRAY_IMAGE,
	BLUR_IMAGE,
	CONTRAST_IMAGE,
	CANNY_IMAGE,
	LAST_STEP
};

// Global variables
Mat guiframe;
bool new_frame = false;
char main_window_name[] = "Team eVision :: Lightning Asystant";
int show_step = 0;
Tracer	processing_tracer;


void send_frame_to_gui(Mat &frame, int step){
	if((!new_frame) && (step == show_step)){
		guiframe = frame;			
		new_frame = 1;
	}
}

void *processing_thread_function(void* unsused)
{
	VideoCapture 	cap(0); // camera interface    
    Mat 			frame;
    
    
    if(!cap.isOpened())  // check if we succeeded
    {
        cout << "Could not open default video device" << endl;
        pthread_exit(NULL);
        
    } else {
    	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    }
    
   	while(1) {
		if( !cap.read(frame) ){
			cout << "Camera was disconected";			
			break;
		}		
		
		send_frame_to_gui(frame, SENSOR_IMAGE);
		processing_tracer.start();
		
		// All processing are done on gray image 
		cvtColor(frame, frame, CV_BGR2GRAY);
		processing_tracer.event("Convert to Gray");
		send_frame_to_gui(frame, GRAY_IMAGE);
		
		// Apply a special blur filter which preserves edges
		medianBlur(frame, frame, 7);
		processing_tracer.event("Median blur");
		send_frame_to_gui(frame, BLUR_IMAGE);
		
		// Increase contrast by distributing color histogram to contain all values
		equalizeHist(frame, frame);
		processing_tracer.event("Equalize histogram");
		send_frame_to_gui(frame, CONTRAST_IMAGE);
		
		// detect edges using histeresys
		Canny(frame, frame, 30, 100);
		processing_tracer.event("Edge detection");
		send_frame_to_gui(frame, CANNY_IMAGE);
	}
	
	pthread_exit(NULL);
}

void *pwm_thread_function(void *unsued){
	GPIO p7(7,"out");

	while(1){
		p7.high();
		usleep(10000);
		p7.low();
		usleep(10000);
	}
	
	pthread_exit(NULL);
}


int main(int argc, char** argv)
{
    pthread_t processing_thread, pwm_thread;
    
    
    // GUI setup
	namedWindow(main_window_name);
	createTrackbar("Show Step", main_window_name, &show_step, LAST_STEP-1);
	
	pthread_create(&processing_thread, NULL, processing_thread_function, NULL);    
	//pthread_create(&pwm_thread, NULL, pwm_thread_function, NULL);
	
	
    while(1){
    
    	while(!new_frame){ } // loop
    	
		imshow(main_window_name, guiframe);
		cout << time(NULL) << endl;
		new_frame = 0;
		
		if(waitKey(30) >= 0) {
			
			break;   
		} 
    }
	
	processing_tracer.end();

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
