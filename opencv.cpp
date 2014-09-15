// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -std=c++11 -shared-libgcc opencv.cpp -o opencv

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <pthread.h>

using namespace cv;
using namespace std;

Mat guiframe;
bool running = 0;


void *gui(void* unsused)
{
   	running = 1;
	imshow("camera", guiframe);
	cout << time(NULL) << endl;
	running = 0;
	pthread_exit(NULL);
}



int main(int argc, char** argv)
{
    VideoCapture cap; // camera interface    
    Mat frame;
    pthread_t gui_thread;
    
    // open default 0 device if no other device as first argument was passed
    if(argc > 1){
    	cap.open(atoi(argv[1]));
    } else {
    	cap.open(0);
    }
    
    if(!cap.isOpened())  // check if we succeeded
    {
        cout << "Could not open default video device" << endl;
        return -1;
    } else {
    	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    }
    
    
	namedWindow("camera",1);
    
    while(1) {
		if( !cap.read(frame) ){
			cout << "Camera was disconected";			
			break;
		}
		
		cvtColor(frame, frame, CV_BGR2GRAY);
		
		if(!running){
			guiframe = frame;
			pthread_create(&gui_thread, NULL, gui, NULL);
		}
		
		if(waitKey(30) >= 0) 
			break;
	}

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
