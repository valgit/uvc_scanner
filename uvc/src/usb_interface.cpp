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
#include "usb_interface.h"
namespace usb {
	interface::interface(io_service_t i)
	: raw_iface_(i), intf_(NULL), is_open_(false)
	{
		// can we get a neat IOUSBInterfaceInterface?
		try {
			get_iointerface();
		} catch (exception& e) {
			// fail. clean up and propagate exception
			IOObjectRelease(raw_iface_);
			throw e;
		}
	}

	interface::~interface() {
		IOObjectRelease(raw_iface_);
		if(is_open_) {
			try {
				close();
			} catch (exception& e) {
				// i dont care
			}
		}
		//(*intf_)->Release(intf_);
	}

	/** get_iointerface. Get a useful IOUSBInterfaceInterface
	 * to our raw interface */
	void interface::get_iointerface() {
		IOCFPlugInInterface					**iodev;
		SInt32								score;
		kern_return_t						kr;

		kr = IOCreatePlugInInterfaceForService(raw_iface_,
					kIOUSBInterfaceUserClientTypeID,
					kIOCFPlugInInterfaceID,
					&iodev,
					&score);
		if(kr || !iodev) {
			throw exception("could not get plugin interface");
		}
		
		kr = (*iodev)->QueryInterface(iodev, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID245),
					(LPVOID*)&intf_);
		if(kr) {
			intf_ = NULL;
			throw exception("could not query interface");
		}
		// cleanup
		IODestroyPlugInInterface(iodev);
		if(!intf_) {
			throw exception("could not get an interface");
		}
	}

	void interface::open() {
		if(!is_open_) {
			kern_return_t kr = (*intf_)->USBInterfaceOpen(intf_);
			if(kr) {
				throw exception("could not open interface");
			}
			is_open_ = true;
		}
	}

	void interface::close() {
		if(is_open_) {
			kern_return_t kr = (*intf_)->USBInterfaceClose(intf_);
			if(kr) {
				throw exception("could not close interface");
			}
			is_open_ = false;
		}
	}

	void interface::set_alternate(uint8_t alternate) {
		kern_return_t kr =
			(*intf_)->SetAlternateInterface(intf_, alternate);
		if(kr) {
			throw exception("SetAlternateInterface failed", kr);
		}
	}

	uint8_t interface::get_number() const {
		uint8_t num;
		kern_return_t kr =
			(*intf_)->GetInterfaceNumber(intf_, &num);
		if(kr) {
			throw exception("GetInterfaceNumber failed");
		}
		return num;
	}

	uint16_t interface::release_number() const {
		kern_return_t kr;
		uint16_t num;
		kr = (*intf_)->GetDeviceReleaseNumber(intf_, &num);
		if(kr) {
			throw exception("could not GetDeviceReleaseNumber");
		}
		return num;
	}

	uint16_t interface::control_request(bool dir_in, uint8_t request,
			uint16_t value, uint16_t index, uint16_t length, void* buffer, uint8_t pipe_ref) const {
		IOUSBDevRequest req;
		req.bmRequestType = USBmakebmRequestType(dir_in?kUSBIn:kUSBOut, kUSBClass, kUSBInterface);
		req.bRequest = request;
		req.wValue = value;
		req.wIndex = index;
		req.wLength = length;
		req.wLenDone = 0;
		req.pData = buffer;

		kern_return_t kr = (*intf_)->ControlRequest(intf_, pipe_ref, &req);
		if(kr!=kIOReturnSuccess) {
			if(kr==kIOUSBPipeStalled) {
				(*intf_)->ClearPipeStall(intf_, pipe_ref);
			}
			return 0;
		}
		return req.wLenDone;
	}

	void interface::clear_stall_both_ends(uint8_t pipe_ref) {
		(*intf_)->ClearPipeStallBothEnds(intf_, pipe_ref);
	}

	kern_return_t interface::read_isoch(uint8_t pipe_ref, void* buffer, uint64_t frame_start, uint32_t num_frames,
			IOUSBIsocFrame* frame_list, IOAsyncCallback1 callback, void* refcon) {
		return (*intf_)->ReadIsochPipeAsync(intf_, pipe_ref, buffer, frame_start, num_frames, frame_list, callback, refcon);
	}


	void interface::async_event_source(CFRunLoopSourceRef* s) {
		(*intf_)->CreateInterfaceAsyncEventSource(intf_, s);
	}


	uint64_t interface::bus_frame_number() {
		uint64_t num;
		AbsoluteTime time;
		kern_return_t ret;
		ret = (*intf_)->GetBusFrameNumber(intf_, &num, &time);
		if(ret) {
			throw exception("GetBusFrameNumber", ret);
		}
		return num;
	}
	
	void interface::abort_pipe(uint8_t pipe_ref) {
		(*intf_)->AbortPipe(intf_, pipe_ref);
	}
};


