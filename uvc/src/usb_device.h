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
#ifndef USB_DEVICE_H__
#define USB_DEVICE_H__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include "usb_exception.h"
#include "usb_interface.h"

namespace usb {
	class device {
		public:
			device(long vendor, long product);
			~device();
			interface* get_interface(	uint16_t iclass = kIOUSBFindInterfaceDontCare,
										uint16_t isubclass = kIOUSBFindInterfaceDontCare,
										uint16_t iproto = kIOUSBFindInterfaceDontCare,
										uint16_t ialternate = kIOUSBFindInterfaceDontCare);
			void* configuration_descriptor(uint16_t& length) const;
		private:
			/* private methods */
			void init_from_vendor_product(long vendor, long product);
			void get_device_interface();
			void clean_up();

			/* members */
			long						vendor_,
										product_;
			io_service_t				device_;				///< the main interface to our device
			IOUSBConfigurationDescriptorPtr
										conf_desc_;				///< configurtion descriptor, managed by IOKit (dont release!)
			IOUSBDeviceInterface245		**device_interface_;	///< the interface to work on
	};
};
#endif /* USB_DEVICE_H__*/

