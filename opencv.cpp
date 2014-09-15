// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -shared-libgcc opencv.cpp -o opencv

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <thread>

using namespace cv;
using namespace std;

void gui(); // thread

Mat bwFrame;

int main(int argc, char** argv)
{
    thread gui_thread;
    VideoCapture cap; // camera interface    
    Mat frame;
    
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
    
    

    
    while(1) {
		if( !cap.read(frame) ){
			cout << "Camera was disconected";			
			break;
		}
		
		cvtColor(frame, bwFrame, CV_BGR2GRAY);
		
		if(!gui_thread.joinable())
			gui_thread = thread(gui);		
		
		if(waitKey(30) >= 0) 
			break;
	}

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

void gui( ){
	
	// startup code
	namedWindow("camera",1);
	
	//loop
	while(1){
		imshow("camera", bwFrame);
		cout << time(NULL) << endl;
	}
}
