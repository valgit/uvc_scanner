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
#ifndef USB_INTERFACE_H__
#define USB_INTERFACE_H__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include "usb_exception.h"

namespace usb {
	class interface {
		public:
			interface(io_service_t i);
			~interface();
			void open();
			void close();
			inline bool is_open() const { return is_open_; }
			void set_alternate(uint8_t alternate);
			uint8_t get_number() const;
			uint16_t release_number() const;
			uint16_t control_request(bool dir_in, uint8_t request,
				uint16_t value, uint16_t index, uint16_t length, void* buffer, uint8_t pipe_ref=0) const;
			void clear_stall_both_ends(uint8_t pipe_ref=0);
			kern_return_t read_isoch(uint8_t pipe_ref, void* buffer, uint64_t frame_start, uint32_t num_frames,
					IOUSBIsocFrame* frame_list, IOAsyncCallback1 callback, void* refcon);
			void async_event_source(CFRunLoopSourceRef *s);
			uint64_t bus_frame_number();
			void abort_pipe(uint8_t pipe_ref);
		private:
			/* methods */
			void get_iointerface();
			/* variables */
			io_service_t						raw_iface_;
			IOUSBInterfaceInterface245			**intf_;
			bool								is_open_;
	};
};

#endif /* USB_INTERFACE_H__ */

