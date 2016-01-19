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
#include "usb_device.h"

namespace usb {
	device::device(long vendor, long product)
	: vendor_(vendor), product_(product), conf_desc_(NULL), device_interface_(NULL)
	{
		/* initialize basic device. if anything goes wrong here,
		 * nothing is left over, so just pass the exception along */
		init_from_vendor_product(vendor, product);
		/* now get everything we need and open up the device for
		 * use. if anything goes wrong here, we need to free
		 * the device we obtained before! */
		try {
			get_device_interface();
		}catch(exception& e) {
			// above failed, so clean up a bit
			IOObjectRelease(device_);
			// and propagate exception...
			throw e;
		}
	}

	device::~device() {
		clean_up();
	}

	/** device_from_vendor_product. Get an io_service_t device
	 * for the given vendor/product pair.
	 * @throws if no device could be found
	 */
	void device::init_from_vendor_product(long vendor, long product) {
		CFMutableDictionaryRef	dict = NULL;
		CFNumberRef				numberRef = 0;
		io_iterator_t			iterator = 0;
		kern_return_t			ret;

		dict = IOServiceMatching(kIOUSBDeviceClassName);
		if(!dict) {
			throw exception("Cannot create dict");
		}

		numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendor_);
		if(!numberRef) { throw exception("CFNumberCreate"); }

		CFDictionaryAddValue(dict, CFSTR(kUSBVendorID), numberRef);
		CFRelease(numberRef);
		numberRef = 0;

		numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &product_);
		if(!numberRef) { throw exception("CFNumberCreate"); }

		CFDictionaryAddValue(dict, CFSTR(kUSBProductID), numberRef);
		CFRelease(numberRef);
		numberRef = 0;

		ret = IOServiceGetMatchingServices(kIOMasterPortDefault, dict, &iterator);
		if(ret) {
			throw exception("IOServiceGetMatchingServices", ret);
		}

		/* select first found device */
		device_ = IOIteratorNext(iterator);

		/* clean up */
		IOObjectRelease(iterator);
		iterator = 0;

		/* check success */
		if(!device_) {
			throw exception("did not find any suitable device");
		}
	}

	void device::get_device_interface() {
		IOCFPlugInInterface						**iodev;
		IOUSBDeviceInterface245					**dev;
		SInt32									score;
		UInt8									num_conf;
		kern_return_t							kr;

		/* from our io_service_t, get a useful IOUSBDeviceInterface245 */
		kr = IOCreatePlugInInterfaceForService(device_,
					kIOUSBDeviceUserClientTypeID,
					kIOCFPlugInInterfaceID,
					&iodev,
					&score);
		if(kr || !iodev) {
			throw exception("Could not create plugin interface");
		}

		kr =  (*iodev)->QueryInterface(iodev,
					CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID245),
					(LPVOID*)&dev);
		IODestroyPlugInInterface(iodev);
		if(kr || !dev) {
			throw exception("Query interface failed");
		}

		/* We now have got a device interface, so open it */
		kr = (*dev)->USBDeviceOpen(dev);
		if(kr) {
			throw exception("opening usb device failed");
		}

		/* work with our device */
		kr = (*dev)->GetNumberOfConfigurations(dev, &num_conf);
		if(kr) {
			(*dev)->USBDeviceClose(dev);
			throw exception("Could not get number of configurations");
		}
		if(!num_conf) {
			(*dev)->USBDeviceClose(dev);
			throw exception("Device as no configurations");
		}

		/* fetch configuration descriptor */
		kr = (*dev)->GetConfigurationDescriptorPtr(dev, 0, &conf_desc_);
		if(kr) {
			(*dev)->USBDeviceClose(dev);
			throw exception("Failed getting configuration descriptor");
		}

		// set config -- is this really necessary?
		kr = (*dev)->SetConfiguration(dev, conf_desc_->bConfigurationValue); 
		if(kr) {
			(*dev)->USBDeviceClose(dev);
			throw exception("failed setting configuration");
		}

		/* all done, save the stuff we need */
		device_interface_ = dev;
	}

	/** get_interface. Returns an interface for the given interface
	 * class, subclass, protocol and alternate setting.
	 * @param iclass device class or kIOUSBFindInterfaceDontCare
	 * @param isubclass device sub class or kIOUSBFindInterfaceDontCare
	 * @param iproto device protocol or kIOUSBFindInterfaceDontCare
	 * @param ialternate device alternate or kIOUSBFindInterfaceDontCare
	 * @return interface wrapper for give interface, or NULL on error
	 */
	interface* device::get_interface(uint16_t iclass, uint16_t isubclass, uint16_t iproto, uint16_t ialternate) {
		IOUSBFindInterfaceRequest				interface_request;
		io_iterator_t							iterator;
		io_service_t							interface_ref;
		interface								*iface = NULL;
		kern_return_t							kr;

		interface_request.bInterfaceClass = iclass;
		interface_request.bInterfaceSubClass = isubclass;
		interface_request.bInterfaceProtocol = iproto;
		interface_request.bAlternateSetting = ialternate;

		kr = (*device_interface_)->CreateInterfaceIterator(device_interface_, &interface_request, &iterator);
		if(kr) {
			throw exception("could not create interface iterator", kr);
		}

		interface_ref=IOIteratorNext(iterator);
		if(interface_ref) {
			// we have found an interface
			try {
				iface = new interface(interface_ref);
			} catch (exception& e) {
				// something went wrong here
				// clean up anyways
				IOObjectRelease(iterator);
				// and propagate exception
				throw e;
			}
		}
		IOObjectRelease(iterator);
		iterator = 0;
		return iface;
	}


	void device::clean_up() {
		IOObjectRelease(device_);
		if(device_interface_) {
			(*device_interface_)->USBDeviceClose(device_interface_);
			(*device_interface_)->Release(device_interface_);
		}
	}

	void* device::configuration_descriptor(uint16_t& length) const {
		length = USBToHostWord(conf_desc_->wTotalLength);
		return (void*)conf_desc_;
	}

};

