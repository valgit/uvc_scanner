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
#ifndef USB_ISOCH_H__
#define USB_ISOCH_H__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <list>
#include <set>
#include <pthread.h>

#include "usb_exception.h"
#include "usb_device.h"
#include "usb_interface.h"

#include "ringbuffer.h"

namespace usb {

	class isoch_transfer;	
	/* private, do not use: */
	struct isoch_request {
		isoch_transfer		*transfer;
		IOUSBIsocFrame		*frame_list;
		uint8_t				*buffer;
		uint64_t			frame_num;
		bool				done;
		bool				ZOMBIE;

		isoch_request(isoch_transfer* tf);
		~isoch_request();
		static void iso_callback(void* refcon, IOReturn result, void* arg0);
	};

	struct isoch_request_cmp {
		bool operator() (const isoch_request* lhs, const isoch_request* rhs) const {
			return lhs->frame_num < rhs->frame_num;
		}
	};
	typedef std::set<isoch_request*, isoch_request_cmp>	request_set;
	
	typedef ringbuffer<isoch_request*> request_rb;

	class isoch_transfer {
		public:
			isoch_transfer(interface* intf, uint32_t pipe_size);
			~isoch_transfer();

			void start();
			void stop();
#if 0
			request_set* start_process();
			void end_process();
			void wait_for_data();
#endif
			inline request_rb* request_ringbuffer() const { return req_done_; }
		
			inline uint32_t num_muframes() const { return num_muframes_; }
			inline uint32_t pipe_size() const { return pipe_size_; }
		private:
			interface				*intf_;
			uint32_t				pipe_size_;
			uint32_t				num_frames_;
			uint32_t				num_muframes_;
			uint32_t				num_reqs_;

			uint64_t				current_frame_;

			bool					running_;
			bool					should_stop_;
			pthread_t				worker_thread_;
			static void*			worker_func(void* vme);

			pthread_mutex_t			mutex_unprocessed_;
#if 0
			pthread_mutex_t			mutex_data_avail_;
			pthread_cond_t			cond_data_avail_;
#endif
			request_set				req_outstanding_;
			//request_set				req_done_;
			request_rb				*req_done_;

			friend class isoch_request;
	};
};
#endif /* USB_ISOCH_H__ */

