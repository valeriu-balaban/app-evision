// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -shared-libgcc opencv.cpp -o opencv

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    VideoCapture cap; // camera interface    
    
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

    Mat frame, bwFrame;
    namedWindow("camera",1);
    
    while(1) {
		if( !cap.read(frame) ){
			cout << "Camera was disconected";
			break;
		}
		
		cvtColor(frame, bwFrame, CV_BGR2GRAY);
		
		imshow("camera", bwFrame);

		if(waitKey(30) >= 0) 
			break;
	}

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
