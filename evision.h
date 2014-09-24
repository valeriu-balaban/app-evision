#ifndef EVISION_H
#define EVISION_H

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

enum Processing_Step{
	SENSOR_IMAGE,
	GRAY_IMAGE,
	CONTRAST_IMAGE,
	BLUR_IMAGE,
	CANNY_IMAGE,
	LAST_STEP
};

void send_frame_to_gui(cv::Mat &frame, int step);

void *processing_thread_function(void* unsused);

void *pwm_thread_function(void *unsued);

void add_info(int fps);

#endif
