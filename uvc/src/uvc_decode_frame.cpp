/*
 Copyright (c) 2010 Max Grosse (max.grosse(at)ioctl.eu)
 
 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 
 3. This notice may not be removed or altered from any source
 distribution.
*/
#include "uvc_decode_frame.h"
#include "uvc_camera.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <sys/time.h>

#include "ringbuffer.h"

namespace uvc {


	struct decode_stats {
		uint32_t	num_frames;
		uint32_t	num_bytes;
		uint32_t	num_bytes_frames;
		
		struct timeval last;
		struct timeval tmp;
		
		uint32_t	kibps_u;
		float		fps;

		decode_stats() : num_frames(0), num_bytes(0), num_bytes_frames(0) {
			gettimeofday(&last, NULL);
		}
		void update_bytes(uint32_t b) {
			num_bytes += b;
		}
		void update_bytes_frames(uint32_t b) {
			num_bytes_frames += b;
		}
		void update_frames(uint32_t f) {
			num_frames += f;

			if(num_frames > 30) {
				gettimeofday(&tmp, NULL);
				long diff_sec = tmp.tv_sec - last.tv_sec;
				long diff_usec = tmp.tv_usec - last.tv_usec;
				double diff_msec = (diff_sec*1000.) + (diff_usec/1000.);			

				fps = (num_frames*1000.)/diff_msec;
				uint32_t kibps = ((num_bytes/(1024.))*1000.)/(diff_msec);
				kibps_u = ((num_bytes_frames/(1024.))*1000.)/(diff_msec);
				printf("** %2.2f fps @ %u KiB/s real, %u KiB/s effective\n", fps, kibps, kibps_u);

				num_frames = 0;
				num_bytes = 0;
				num_bytes_frames = 0;
				gettimeofday(&last, NULL);
			}
		}
	};


	decode_frame::decode_frame(camera *cam)
	: cam_(cam)
	{
		transfer_ = cam->transfer_;
		running_ = false;
		if(pthread_mutex_init(&mutex_frame_, NULL)) {
			throw "YUCK";
		}
		uint32_t frame_size =cam_->width() * cam_->height() * cam_->bpp() / 8;
		current_frame_ = new uint8_t[frame_size];
	}

	decode_frame::~decode_frame() {
		pthread_mutex_lock(&mutex_frame_);
		pthread_mutex_destroy(&mutex_frame_);
	}

	void decode_frame::start_decode() {
		if(running_) {
			printf("already decoding!\n");
			return;
		}
		running_ = true;
		if(pthread_create(&worker_thread_, NULL, decode_frame::worker_func, this)) {
			printf("decode_frame thread launch FAILED!\n");
			running_ = false;
		}
	}
	void decode_frame::end_decode() {
		running_ = false;
		pthread_join(worker_thread_, NULL);
	}

	// request a frame
	uint8_t* decode_frame::get_frame() {
		uint32_t frame_size =cam_->width() * cam_->height() * cam_->bpp() / 8;
		uint8_t *newframe = new uint8_t[frame_size];
		pthread_mutex_lock(&mutex_frame_);
		memcpy(newframe, current_frame_, frame_size);
		pthread_mutex_unlock(&mutex_frame_);
		return newframe;
	}

	void* decode_frame::worker_func(void* vme) {
		decode_frame *me = (decode_frame*)vme;
		//printf("START DECODING PACKETS! ****\n");
		decode_stats stats;

		uint32_t frame_size = me->cam_->width() * me->cam_->height() * me->cam_->bpp() / 8;
		uint8_t *v_frame = new uint8_t[frame_size];
		uint8_t *fp = v_frame;
		uint8_t fid = 2;
		uint32_t num_muframes = me->transfer_->num_muframes();
		uint32_t bytes_done = 0;

		///uint32_t frame_count = 0;
#if 0
		usb::request_set *his_reqs;
#endif
		usb::request_rb *his_reqs = me->transfer_->request_ringbuffer();
		
		///bool buf_active_ = false;
#if 0
		while(me->running_) {
#else
		while(me->running_ && !his_reqs->done()) {
#endif
			
#if 0
			/* Wait for new data to become available */
			me->transfer_->wait_for_data();

			/* copy all requests and clear "his" queue */
			his_reqs=me->transfer_->start_process();
			if(his_reqs->empty()) {
				//most likely we are done now...
				me->transfer_->end_process();
				continue;
			}
			for(usb::request_set::iterator i=his_reqs->begin(); i!=his_reqs->end(); ++i) {
				usb::isoch_request *req = (*i);
				me->to_process_.insert(req);
			}
			his_reqs->clear();
			me->transfer_->end_process();

			printf("** To Process: %d frames\n", me->to_process_.size());
			/* TODO:
			 * possibly clear/drop a hole bunch of frames if we couldn't
			 * keep up with processing, e.g. to_process_.size() > XXX ?
			 */
			/* Process all my requests */
			for(usb::request_set::iterator i=me->to_process_.begin(); i!=me->to_process_.end(); ++i) {
				usb::isoch_request *req = (*i);
#endif
			{
				usb::isoch_request *req = his_reqs->pop();
				
				//printf("REQ @%lld\n",req->frame_num);
				for(int j=0; j<num_muframes; ++j) {
					IOUSBIsocFrame &isoframe = req->frame_list[j];
					if(isoframe.frStatus != kIOReturnSuccess && isoframe.frStatus != kIOReturnUnderrun) {
						continue;
					}
					// empty frames?
					if(isoframe.frActCount==0) {
						continue;
					}
					// ok, valid frame, process!

					uint8_t *buf = &req->buffer[me->transfer_->pipe_size()*j];
					uint32_t len = isoframe.frActCount;
					uint8_t h_len = buf[0];
					uint8_t flags = buf[1];
					if(flags&(1<<6)) {
						//error bit set, skip this frame
						continue;
					}
					
					/** stats... **/
					stats.update_bytes(isoframe.frActCount-h_len);
					me->fps_ = stats.fps;
					me->kibps_ = stats.kibps_u;
					
					//printf("..valid frame..(%d bytes)\n",len);
					//if(len>12) { printf("DATAFRAME!\n"); }
					uint8_t fid_new = flags&(1<<0);

					if(fid_new!=fid && (len-h_len)>0) {
						//printf("NEXT FRAME (%d = %0.02f done)\n", bytes_done, bytes_done/float(frame_size));
						// next frame!
						if(bytes_done == frame_size) {
							stats.update_bytes_frames(bytes_done);
							stats.update_frames(1);
							//printf("* COMPLETED FRAME! *\n");
							pthread_mutex_lock(&(me->mutex_frame_));
							memcpy(me->current_frame_, v_frame, frame_size);
							pthread_mutex_unlock(&(me->mutex_frame_));
						}
						bytes_done = 0;
						fp = v_frame;
					}
					// continue writing
					len -= buf[0];
					buf += buf[0];
					if(len > (frame_size-bytes_done)) {
						//printf("ARGH?! Overrun! %u/%u bytes\n", len,frame_size-bytes_done);
						continue;
					}
					memcpy(fp, buf, len);
					bytes_done += len;
					fp += len;
					fid = fid_new;
				}

				usleep(100*(rand()%20));
				/* request frame finished: */
				delete req;
			}
#if 0
			/* all requests processed, clean */
			me->to_process_.clear();
#endif
		}
		//printf("STOP DECODING PACKETS! ****\n");
		delete [] v_frame;
		return NULL;
	}
};

