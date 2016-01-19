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
#include "usb_prober.h"
#include "usb_exception.h"
#include <iostream>

namespace usb {
	
	uint8_t* get_descriptor_request(IOUSBDeviceInterface **intf, uint8_t idx) {
		IOReturn            kr;
		
		/* first request to query length */
		uint8_t length;
		
		IOUSBDevRequest req;
		req.bmRequestType = 0x80;
		req.bRequest = kUSBRqGetDescriptor;
		req.wValue = (kUSBStringDesc<<8)+idx;
		req.wIndex = 0x409; //english
		req.wLength = 1;
		req.wLenDone = 0;
		req.pData = &length;//buffer;
		
		kr = (*intf)->DeviceRequest(intf, &req);
		if(kr!=kIOReturnSuccess) {
			printf("name request failed\n");
			return NULL;
		}

		/* then, query value */
		unsigned char *buffer = new unsigned char[length+1]; 
		bzero(buffer, length+1);
		req.bmRequestType = 0x80;
		req.bRequest = kUSBRqGetDescriptor;
		req.wValue = (kUSBStringDesc<<8)+idx;
		req.wIndex = 0x409; //english
		req.wLength = length;
		req.wLenDone = 0;
		req.pData = buffer;
		kr = (*intf)->DeviceRequest(intf, &req);
		if(kr!=kIOReturnSuccess) {
			printf("name request failed\n");
			return NULL;
		}
#if 0
		for(int i=1; i<length; ++i) {
			unsigned char c = buffer[i];
			if(c<20||c>0x7f) printf("."); //printf("\\x%02x",(int)c);
			else printf("%c",c);
		}
		printf("|END\n");
#endif
		buffer[1]=0; ///< I don't know...
		return buffer;
	}
	
	devices_of_class_enumerator::devices_of_class_enumerator(uint8_t classid) {
		CFDictionaryRef 	matchingDict = NULL;
		mach_port_t         mMasterDevicePort = MACH_PORT_NULL;
		io_iterator_t       devIter = IO_OBJECT_NULL;
		io_service_t        ioDeviceObj	= IO_OBJECT_NULL;
		IOReturn            kr;

		kr = IOMasterPort(MACH_PORT_NULL, &mMasterDevicePort);
		if(kr != kIOReturnSuccess) {
			throw exception("IOMaterPort failed", kr);
		}

		matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
		if(matchingDict == NULL) {
			mach_port_deallocate(mach_task_self(), mMasterDevicePort);
			throw exception("IOServiceMatching failed");
		}

		kr = IOServiceGetMatchingServices(mMasterDevicePort, matchingDict, &devIter);
		if(kr != kIOReturnSuccess) {
			mach_port_deallocate(mach_task_self(), mMasterDevicePort);
			throw exception("IOServiceGetMatchingServices failed", kr);
		}

		while (ioDeviceObj = IOIteratorNext(devIter)) {
			IOCFPlugInInterface 	**ioPlugin;
			IOUSBDeviceInterface	**deviceIntf = NULL;
			SInt32                  score;
			bool					uvc_device = false;

			kr = IOCreatePlugInInterfaceForService(ioDeviceObj, kIOUSBDeviceUserClientTypeID, 
					kIOCFPlugInInterfaceID, &ioPlugin, &score);
			if (kr != kIOReturnSuccess) {
				IOObjectRelease(ioDeviceObj);
				continue;
			}

			kr = (*ioPlugin)->QueryInterface(ioPlugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID *)&deviceIntf);
			IODestroyPlugInInterface(ioPlugin);
			ioPlugin = NULL;

			if (kr != kIOReturnSuccess) {
				IOObjectRelease(ioDeviceObj);
				continue;
			}

			uint16_t product, vendor;
			kr = (*deviceIntf)->GetDeviceProduct(deviceIntf, &product);
			if(kr!=kIOReturnSuccess) {
				(*deviceIntf)->Release(deviceIntf);
				IOObjectRelease(ioDeviceObj);
				continue;
			}
			kr = (*deviceIntf)->GetDeviceVendor(deviceIntf, &vendor);
			if(kr!=kIOReturnSuccess) {
				(*deviceIntf)->Release(deviceIntf);
				IOObjectRelease(ioDeviceObj);
				continue;
			}
			uint8_t devclass;
			kr = (*deviceIntf)->GetDeviceClass(deviceIntf, &devclass);
			if(kr!=kIOReturnSuccess) {
				(*deviceIntf)->Release(deviceIntf);
				IOObjectRelease(ioDeviceObj);
				continue;
			}

			if(devclass==9 /* Hub */) {
				(*deviceIntf)->Release(deviceIntf);
				IOObjectRelease(ioDeviceObj);
				continue;
			}
			
			uint8_t *prod_str=NULL, *manu_str=NULL, idx;
			IOUSBDeviceInterface182	*deviceIntf182 = (IOUSBDeviceInterface182*)*deviceIntf;
			kr = deviceIntf182->USBGetManufacturerStringIndex(deviceIntf, &idx);
			if(kr!=kIOReturnSuccess) {
				manu_str = NULL;
			} else {
				manu_str = get_descriptor_request(deviceIntf, idx);
			}
			
			kr = deviceIntf182->USBGetProductStringIndex(deviceIntf, &idx);
			if(kr!=kIOReturnSuccess) {
				prod_str = NULL;
			} else {
				prod_str = get_descriptor_request(deviceIntf, idx);
			}

			/* Walk all possible interfaces, check for UVC Class! */
			{
				IOUSBFindInterfaceRequest interface_request;
				io_iterator_t iter;
				io_service_t inter_ref;

				interface_request.bInterfaceClass = 0x0e; // UVC Class
				interface_request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
				interface_request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
				interface_request.bAlternateSetting = kIOUSBFindInterfaceDontCare;

				kr = (*deviceIntf)->CreateInterfaceIterator(deviceIntf, &interface_request, &iter);
				if(kr!=kIOReturnSuccess) {
					(*deviceIntf)->Release(deviceIntf);
					IOObjectRelease(ioDeviceObj);
					continue;
				}

				for(inter_ref=IOIteratorNext(iter); (void*)inter_ref!=NULL; inter_ref=IOIteratorNext(iter)) {
					uvc_device = true;
					break;
				}

				IOObjectRelease(iter);
			}

			(*deviceIntf)->Release(deviceIntf);
			IOObjectRelease(ioDeviceObj);

			if(uvc_device) {
				//printf("uvc device found: %04x/%04x\n", vendor, product);
				//devs_.push_back(vendor_product_pair(vendor,product));
				devs_.push_back(new device_info_t(vendor,product,manu_str,prod_str));
			} else {
				if(manu_str) delete [] manu_str;
				if(prod_str) delete [] prod_str;
			}
		}

		IOObjectRelease(devIter);
		mach_port_deallocate(mach_task_self(), mMasterDevicePort);	
	}
	devices_of_class_enumerator::~devices_of_class_enumerator() {
		device_info_list::iterator iter = devs_.begin();
		for(;iter!=devs_.end();++iter) {
			delete (*iter);
		}
		devs_.clear();
	}
};

