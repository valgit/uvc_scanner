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
#include "uvc_control.h"
#include "uvc_types.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

namespace uvc {
	/* controls */
	control::control(usb::interface *intf, confdesc* desc)
	: intf_(intf), desc_(desc) {
	}

	control::~control() {
	}

	template<typename T>
	bool control::control_request(uint8_t request, uint8_t unit, uint8_t selector, T* buffer) const {
		uint16_t len = sizeof(T);
		uint8_t intf_num = intf_->get_number();
		uint16_t ret = intf_->control_request(
				(request&0x80),
				request,
				(unit<<8),
				(selector<<8)|intf_num,
				len,
				buffer);
		return len==ret;
	}

	bool control::auto_exposure() const {
		uint8_t val;
		if(control_request(GET_CUR, CT_AE_MODE_CONTROL, desc_->terminal_unit_id(), &val)) {
			return val&0x08;
		}
		return false;
	}
	bool control::auto_exposure(bool set) {
		uint8_t val = set?0x08:0x01;
		if(control_request(SET_CUR, CT_AE_MODE_CONTROL, desc_->terminal_unit_id(), &val)) {
			return val&0x08;
		}
		return false; ///TODO: output error?
	}
	void control::exposure_minmax(long& emin, long& emax) const {
		if(!control_request(GET_MIN, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, desc_->terminal_unit_id(), &emin)) {
			emin = emax = -1;
			return; ///TODO: report error
		}
		if(!control_request(GET_MAX, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, desc_->terminal_unit_id(), &emax)) {
			emin = emax = -1;
			return; ///TODO: report error
		}
		emin = USBToHostLong(emin);
		emax = USBToHostLong(emax);
	}
	long control::exposure_resolution() const {
		long val;
		if(!control_request(GET_RES, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, desc_->terminal_unit_id(), &val)) {
			return -1; ///TODO: report error
		}
		val = USBToHostLong(val);
		return val;
	}
	long control::exposure() const {
		long val;
		if(!control_request(GET_CUR, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, desc_->terminal_unit_id(), &val)) {
			return -1; ///TODO: report error
		}
		val = USBToHostLong(val);
		return val;
	}
	long control::exposure(long set) {
		set = HostToUSBLong(set);
		if(!control_request(SET_CUR, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, desc_->terminal_unit_id(), &set)) {
			return -1; ///TODO: report error
		}
		return USBToHostLong(set);
	}


	/***
	 * Backlight Compensation
	 **/

	bool control::backlight_minmax(uint16_t& emin, uint16_t& emax) const {
		if(!control_request(GET_MIN, PU_BACKLIGHT_COMPENSATION_CONTROL, desc_->processing_unit_id(), &emin)) {
			emin = emax = -1;
			return false;
		}
		if(!control_request(GET_MAX, PU_BACKLIGHT_COMPENSATION_CONTROL, desc_->processing_unit_id(), &emax)) {
			emin = emax = -1;
			return false;
		}
		emin = USBToHostWord(emin);
		emax = USBToHostWord(emax);
		return true;
	}
	uint16_t control::backlight() const {
		uint16_t val;
		if(!control_request(GET_CUR, PU_BACKLIGHT_COMPENSATION_CONTROL, desc_->processing_unit_id(), &val)) {
			return -1; ///TODO: report error
		}
		val = USBToHostWord(val);
		return val;
	}
	uint16_t control::backlight(uint16_t set) {
		set = HostToUSBWord(set);
		if(!control_request(SET_CUR, PU_BACKLIGHT_COMPENSATION_CONTROL, desc_->processing_unit_id(), &set)) {
			return -1; ///TODO: report error
		}
		return USBToHostWord(set);
	}

	/**
	 * gain
	 ***/

	bool control::gain_minmax(uint16_t& emin, uint16_t& emax) const {
		if(!control_request(GET_MIN, PU_GAIN_CONTROL, desc_->processing_unit_id(), &emin)) {
			emin = emax = -1;
			return false;
		}
		if(!control_request(GET_MAX, PU_GAIN_CONTROL, desc_->processing_unit_id(), &emax)) {
			emin = emax = -1;
			return false;
		}
		emin = USBToHostWord(emin);
		emax = USBToHostWord(emax);
		return true;
	}
	uint16_t control::gain() const {
		uint16_t val;
		if(!control_request(GET_CUR, PU_GAIN_CONTROL, desc_->processing_unit_id(), &val)) {
			return -1; ///TODO: report error
		}
		val = USBToHostWord(val);
		return val;
	}
	uint16_t control::gain(uint16_t set) {
		set = HostToUSBWord(set);
		if(!control_request(SET_CUR, PU_GAIN_CONTROL, desc_->processing_unit_id(), &set)) {
			return -1; ///TODO: report error
		}
		return USBToHostWord(set);
	}


	/**
	 * power-line
	 * 0: disabled
	 * 1: 50Hz
	 * 2: 60Hz
	 ***/

	uint8_t control::powerline() const {
		uint8_t val;
		if(!control_request(GET_CUR, PU_POWER_LINE_FREQUENCY_CONTROL, desc_->processing_unit_id(), &val)) {
			return -1; ///TODO: report error
		}
		return val;
	}
	uint8_t control::powerline(uint8_t set) {
		if(!control_request(SET_CUR, PU_POWER_LINE_FREQUENCY_CONTROL, desc_->processing_unit_id(), &set)) {
			return -1; ///TODO: report error
		}
		return set;
	}
}

