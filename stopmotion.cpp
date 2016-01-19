/**
 * stopmotion.cpp
 * - Simple application for recording stop-motion movies, frame-by-frame.
 *
 * Uses my custom UVC Webcam driver for MacOS X,
 * see http://svn.ioctl.eu/mac_uvc
 *
 * For easy window handling and image processing,
 * the OpenCV library is used, Version 2.1,
 * see http://opencv.willowgarage.com
 *
 * 
 *
 * Copyright (c) 2010 max.grosse - http://ioctl.eu
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 **/ 

#include <cstdio>
#include <cstdlib>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <uvc/uvc_camera.h>
#include <uvc/uvc_colorcvt.h>

const int full_w = 1280;
const int full_h = 1024;


int preview() {
	/* preview */
	cvNamedWindow("preview", 0);
	int cnt = 0;
	char name[255];
	while(true) {
		sprintf(name, "frames/frame%04d.png", cnt++);
		IplImage *tmp = cvLoadImage(name, 1);
		if(!tmp) {
			printf("** %s not found\n", name);
			break;
		}
		cvShowImage("preview", tmp);
		cvResizeWindow("preview", 800, 600);
		cvWaitKey(83); ///12fps
		cvReleaseImage(&tmp);
	}
	cvWaitKey(250);
	cvDestroyWindow("preview");
	printf("*** Last frame found: %d\n", cnt-1);
	return cnt-1;
}

void start_stream() {
	int frame_number = preview();
	cvNamedWindow("stream", 0);
	IplImage *snapshot = cvCreateImage(cvSize(full_w,full_h), IPL_DEPTH_8U, 3);
	IplImage *snap_small = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 3);
	IplImage *o_tmp = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 3);

	bool overlay = true;

	try {
		uvc::camera *cam = new uvc::camera(0);
		if(!cam->set_config(640,480,16,15)) {
		//if(!cam->set_config(1280,1024,16,5)){
			printf("setconfig failed\n");
			delete cam;
			return;
		}
		IplImage *img = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 3);
		cam->begin_capture();
		while(true) {
			uint8_t *frame = cam->get_frame();
			if(!frame) {
				break;
			}
		   	uvc::yuv422_to_rgb(img->imageData, frame, 640, 480, img->widthStep);	
			cvCvtColor(img, img, CV_RGB2BGR);
			delete [] frame;

			/* display! */
			if(!overlay) {
				cvShowImage("stream", img);
			} else {
				cvAbsDiff(img, snap_small, o_tmp);
				cvAdd(img, o_tmp, o_tmp);
				cvShowImage("stream", o_tmp);
			}


			int k = cvWaitKey(10);
			if(k==27) {
				break;
			}
			if(k==(int)'p') {
				frame_number = preview();
			}
			if(k==(int)'o') {
				overlay ^= 1;
			}
			if(k==32) {
				//snapshot!
				cam->end_capture();
				if(!cam->set_config(full_w,full_h,16,8)){
					printf("setconfig failed\n");
					delete cam;
					return;
				}
				cam->begin_capture();
				sleep(1);
				//snap 10 dummy frames
				for(int i=0; i<5; ++i) {
					uint8_t *snap = cam->get_frame();
					delete [] snap;
					usleep(50*1000);
				}
				uint8_t *snap = cam->get_frame();
				uvc::yuv422_to_rgb(snapshot->imageData, snap, full_w, full_h, snapshot->widthStep);	
				cvCvtColor(snapshot, snapshot, CV_RGB2BGR);
				delete [] snap;

				cvResize(snapshot, snap_small, CV_INTER_NN);

				cvNamedWindow("Last Frame", 0);
				cvShowImage("Last Frame", snapshot);
				cvResizeWindow("Last Frame", 800, 600);
				{
					char name[255];
					sprintf(name, "frames/frame%04d.png", frame_number++);
					cvSaveImage(name, snapshot);
				}
				// return to original resolution
				cam->end_capture();
				if(!cam->set_config(640,480,16,15)){
					printf("setconfig failed\n");
					delete cam;
					return;
				}
				cam->begin_capture();
			}
		}
		delete cam;
	}catch(std::exception& e) {
		printf("OUCH! %s\n", e.what());
	}
	cvDestroyWindow("stream");
}

int main(int argc, char* argv[]) {
	start_stream();
	return 0;
}

