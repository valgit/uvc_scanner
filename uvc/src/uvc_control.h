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
#ifndef UVC_CONTROL_H__
#define UVC_CONTROL_H__
#include "usb_interface.h"
#include "uvc_confdesc.h"
namespace uvc {
	class control {
		public:
			control(usb::interface *intf, confdesc* desc);
			~control();

			bool auto_exposure() const;
			bool auto_exposure(bool set);

			/** exposure:
			 * values in 0.0001 seconds (times 100ns) */
			void exposure_minmax(long& emin, long& emax) const;
			long exposure_resolution() const;
			long exposure() const;
			long exposure(long set);

			bool backlight_minmax(uint16_t& emin, uint16_t& emax) const;
			uint16_t backlight() const;
			uint16_t backlight(uint16_t set);


			bool gain_minmax(uint16_t& emin, uint16_t& emax) const;
			uint16_t gain() const;
			uint16_t gain(uint16_t set);

			/** powerline. Get Poweline-Compensation mode.
			 * 0 - disabled
			 * 1 - 50 Hz
			 * 2 - 60 Hz
			 * @return powerline compensation mode
			 **/
			uint8_t powerline() const;
			/** powerline. Set Poweline-Compensation mode.
			 * 0 - disabled
			 * 1 - 50 Hz
			 * 2 - 60 Hz
			 * @param set mode to set
			 * @return powerline compensation mode
			 **/
			uint8_t powerline(uint8_t set);
		private:
			usb::interface		*intf_;
			confdesc			*desc_;

			template<typename T>
			bool control_request(uint8_t request, uint8_t unit, uint8_t selector, T* buffer) const;

	};
};
#endif /* UVC_CONTROL_H__*/

