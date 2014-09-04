// arm-poky-linux-gnueabi-gcc -lopencv_video -lopencv_core -lopencv_highgui -lopencv_imgproc -lstdc++ -shared-libgcc opencv.cpp -o opencv

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
    {
        cout << "Could not open default video device" << endl;
        return -1;
    }

    Mat frame;
    cap >> frame; // get a new frame from camera

	vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    
    imwrite("test.png", frame, compression_params);

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
