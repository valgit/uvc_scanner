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
#ifndef USB_PROBER_H__
#define USB_PROBER_H__

#include <mach/mach_port.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USB.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/IOCFPlugIn.h>
#include <vector>

namespace usb {
	struct device_info_t {
		uint16_t	vendor,
					product;
		
		uint8_t		*manufacturer_string; ///utf16
		uint8_t		*product_string; ///utf16
		
		device_info_t(uint16_t vendor, uint16_t product, uint8_t *ms, uint8_t *ps)
		: vendor(vendor), product(product), manufacturer_string(ms), product_string(ps) {}
		
		~device_info_t() {
			printf("free device_info_t\n");
			if(manufacturer_string) delete [] manufacturer_string;
			if(product_string) delete [] product_string;
		}
	};
	typedef std::vector<device_info_t*> device_info_list;

	class devices_of_class_enumerator {
		public:
			devices_of_class_enumerator(uint8_t classid);
			~devices_of_class_enumerator();

			inline unsigned count() const { return devs_.size(); }

			inline void get(int idx, uint16_t& vendor, uint16_t& product) const {
				vendor = devs_[idx]->vendor;
				product = devs_[idx]->product;
			}

		inline uint8_t* get_manufacturer_utf16(int idx) const {
			return devs_[idx]->manufacturer_string;
		}
		inline uint8_t* get_product_utf16(int idx) const {
			return devs_[idx]->product_string;
		}
		private:
			device_info_list		devs_;
	};
};
#endif /* USB_PROBER_H__ */

