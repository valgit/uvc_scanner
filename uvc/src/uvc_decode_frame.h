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
#ifndef UVC_DECODE_FRAME_H__
#define UVC_DECODE_FRAME_H__
#include <pthread.h>
#include "usb_isoch.h"
//#include "uvc_camera.h"
namespace uvc {
	class camera;
	class decode_frame {
		public:
			decode_frame(camera *cam);
			~decode_frame();

			void start_decode();
			void end_decode();

			uint8_t* get_frame();
		
			inline float fps() const { return fps_; }
			inline uint32_t kibps() const { return kibps_; }
		private:
			camera					*cam_;
			usb::isoch_transfer		*transfer_;

			bool					running_;
			pthread_t				worker_thread_;
			static void*			worker_func(void* vme);

			pthread_mutex_t			mutex_frame_;
			uint8_t					*current_frame_;
#if 0
			usb::request_set		to_process_;
#endif	
			float					fps_;
			uint32_t				kibps_;
	};	
};
#endif /* UVC_DECODE_FRAME_H__ */

