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
#ifndef UVC_CAMERA_H__
#define UVC_CAMERA_H__

#include "usb_device.h"
#include "usb_interface.h"
#include "usb_isoch.h"
#include "uvc_confdesc.h"
#include "uvc_types.h"
#include "uvc_decode_frame.h"
#include "uvc_control.h"

namespace uvc {
	class camera {
		public:
			camera(uint16_t vendor, uint16_t product);
			camera(unsigned number);
			~camera();
#if 0
			bool set_config(uint16_t width, uint16_t height, uint8_t bpp, uint8_t fps);
#endif
			bool set_config(unsigned idx);
			void begin_capture();
			void end_capture();

			inline uint16_t width() const { return width_; }
			inline uint16_t height() const { return height_; }
			inline uint8_t bpp() const { return bpp_; }
			uint8_t* get_frame();

			inline control& ctl() const { return *control_; }
		
			struct config_t {
				uint8_t fmt_number;
				uint8_t frm_number;
				
				uint16_t width;
				uint16_t height;
				uint8_t	bpp;
				uint32_t interval;
				float	interval_fps;
			};
			typedef std::vector<config_t*> configs_t;
			inline const configs_t* configurations() const { return &configs_; }
		
			inline unsigned num_configs() const { return configs_.size(); }
			inline const config_t* config(unsigned idx) const { return configs_[idx]; }
		
			float fps() const;
			uint32_t kibps() const;
		private:
			/* variables */
			usb::device			*device_;
			confdesc			*confdesc_;
			usb::interface		*if_stream_;

			bool				negotiated_;
			probe				cur_probe_;

			usb::isoch_transfer	*transfer_;
			decode_frame		*decoder_;

			uint16_t			width_;
			uint16_t			height_;
			uint8_t				bpp_;

			usb::interface		*if_control_;
			control				*control_;
		
			configs_t			configs_;
			/* private members */
			void init_with_vendor_product(uint16_t vendor, uint16_t product);

			/** vs_control_request. Video Stream Interface control
			 * request.
			 */
			uint16_t vs_control_request(uint8_t request, uint8_t selector, uint8_t unit,
					uint8_t intf_num, uint16_t length, void* buffer);
			void dbg_print_probe(probe& p);

			friend class decode_frame;
	};
};
#endif /* UVC_CAMERA_H__ */

