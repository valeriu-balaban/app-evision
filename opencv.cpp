// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -std=c++11 -shared-libgcc opencv.cpp -o opencv

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <pthread.h>

using namespace cv;
using namespace std;

Mat guiframe;
bool new_frame = 0;


void *gui(void* unsused)
{
	VideoCapture cap(0); // camera interface    
    Mat frame;
    
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
		
		cvtColor(frame, frame, CV_BGR2GRAY);
		
		if(!new_frame){
			guiframe = frame;
			new_frame = 1;
		}
	}
	
	pthread_exit(NULL);
}



int main(int argc, char** argv)
{
    pthread_t gui_thread;
    
	namedWindow("camera",1);
	
	pthread_create(&gui_thread, NULL, gui, NULL);
    
    while(1){
    
    	while(!new_frame){ } // loop
    	
		imshow("camera", guiframe);
		cout << time(NULL) << endl;
		new_frame = 0;
		
		if(waitKey(30) >= 0) 
			break;    
    }

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
