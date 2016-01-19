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
#include <vector>
#include "usb_prober.h"
#include "uvc_camera.h"
#include "uvc_types.h"

namespace { /* anonymous, local and debug */
	bool very_verbose = false;
};

namespace uvc {
	camera::camera(uint16_t vendor, uint16_t product)
    : negotiated_(false), transfer_(NULL)
	{
		init_with_vendor_product(vendor, product);
	}

	camera::camera(unsigned idx)
    : negotiated_(false), transfer_(NULL)
	{
		usb::devices_of_class_enumerator enu(0xe);
		if(idx>=enu.count()) {
			throw std::out_of_range("index out of range");
		}
		uint16_t vendor, product;
		enu.get(idx, vendor, product);
		init_with_vendor_product(vendor, product);
	}

	void camera::init_with_vendor_product(uint16_t vendor, uint16_t product) {
		device_ = new usb::device(vendor, product);
		
		/* parse configuration descriptor */ {
			uint16_t len;
			uint8_t *data = (uint8_t*)device_->configuration_descriptor(len);
			confdesc_ = new confdesc(data, len);
		}

		/* open streaming interface */
		try {
			if_stream_ = device_->get_interface(INTERFACE_CLASS_VIDEO, INTERFACE_SUBCLASS_STREAMING);
		} catch (std::exception* e) {
			delete device_;
			// propagate
			throw e;
		}

		/* get control interface */
		try {
			if_control_ = device_->get_interface(INTERFACE_CLASS_VIDEO, INTERFACE_SUBCLASS_CONTROL);
			if_control_->open();
			//if_control_->clear_stall_both_ends();
			//if_control_->set_alternate(0);
			control_ = new control(if_control_, confdesc_);
		} catch (std::exception* e) {
			delete if_stream_;
			delete device_;
			// propagate
			throw e;
		}

		/* Enumerate valid configurations */
		try {
			for(uint8_t i=0; i<confdesc_->num_formats(); ++i) {
				video_format_t *fmt = (*confdesc_)[i];
				std::vector<video_frame_t*>::iterator iter = fmt->frames.begin();
				for(;iter!=fmt->frames.end();++iter) {
					video_frame_t &frm = * (*iter);
					if(frm.bFrameIntervalType) {
						float fps = 10000000.f/float(frm.dwFrameIntervals[i]);
						//printf("%dx%dx%d @%f %d\n",frm.wWidth,frm.wHeight,fmt->bBitsPerPixel,fps,frm.dwFrameIntervals[i]);
						config_t *conf = new config_t;
						conf->fmt_number = fmt->number;
						conf->frm_number = frm.bFrameIndex;
						conf->width = frm.wWidth;
						conf->height = frm.wHeight;
						conf->bpp = fmt->bBitsPerPixel;
						conf->interval = frm.dwFrameIntervals[i];
						conf->interval_fps = fps;
						configs_.push_back(conf);
					} else {
						printf("Continuus FPS unsupported for now :(\n");
					}
				}
			}
		} catch (std::exception &e) {
			// propagate
			throw e;
		}
	}

	camera::~camera() {
		if(transfer_) {
			end_capture();
		}
		delete if_stream_;
		delete control_;
		if_control_->close();
		delete if_control_;
		delete device_;
		
		configs_t::iterator configiter = configs_.begin();
		for(;configiter!=configs_.end();++configiter) {
			delete *configiter;
		}
	}

	uint16_t camera::vs_control_request(uint8_t request, uint8_t selector, uint8_t unit,
					uint8_t intf_num, uint16_t length, void* buffer)
	{
		return if_stream_->control_request(
					(request&0x80),
					request,
					(selector<<8),
					(unit<<8)|intf_num,
					length,
					buffer);
	}
#if 0
	bool camera::set_config(uint16_t width, uint16_t height, uint8_t bpp, uint8_t fps) {
		/* find matching format */
		uint8_t format_idx = 0;
		
		video_format_t	*fmt = NULL;
		video_frame_t	*frm = NULL;
		uint32_t		frame_interval = 0;

		while(format_idx < confdesc_->num_formats()) {
			video_format_t *i = (*confdesc_)[format_idx];
			if(i->bBitsPerPixel==bpp) { //at least bpp
				fmt = i;
				break;
			}
			format_idx ++;
		}
		if(!fmt) {
			printf("no matching format\n");
			return false;
		}
		
		/* find matching frame */
		for(std::vector<video_frame_t*>::iterator i=fmt->frames.begin(); i!=fmt->frames.end(); ++i) {
			if((*i)->wWidth==width && (*i)->wHeight==height) {
				frm = *i;
				break;
			}
		}
		if(!frm) {
			printf("no matching size\n");
			return false;
		}

		/* select matching fps */
		if(frm->bFrameIntervalType==0) {
			//TODO
		} else {
			for(int i=0; i<frm->bFrameIntervalType; ++i) {
				float fps_f = 10000000.f/float(frm->dwFrameIntervals[i]);
				uint8_t fps_i = uint8_t(fps_f+0.5);
				if(fps_i==fps) {
					frame_interval = frm->dwFrameIntervals[i];
					break;
				}
			}
		}
		if(!frame_interval) {
			printf("no matching fps\n");
			return false;
		}
	
		if(very_verbose) {	
			printf("FOUND CONFIG:\n");
			frm->print();
			printf("> interval: %d\n",frame_interval);
		}

		/* negotiate */
		if(!if_stream_->is_open()) {
			if_stream_->open();
		}
		// be safe:
		if_stream_->clear_stall_both_ends();
		// set alternate 0 = config
		if_stream_->set_alternate(0);
		if(very_verbose) {
			printf("** VDC=0x%04x\n", confdesc_->vdc());
		}

		/* set up, what we want */
		probe probe_want;
		memset(&probe_want, 0, sizeof(probe));
		probe_want.bmHint = (1); /// keep frame interval
		probe_want.bFormatIndex = fmt->number;
		probe_want.bFrameIndex = frm->bFrameIndex;
		probe_want.dwFrameInterval = HostToUSBLong( frame_interval );
#endif
	bool camera::set_config(unsigned idx) {
		//printf("+++ set_config(%d)\n",idx);
		
		/* negotiate */
		if(!if_stream_->is_open()) {
			if_stream_->open();
		}
		// be safe:
		if_stream_->clear_stall_both_ends();
		// set alternate 0 = config
		if_stream_->set_alternate(0);
		if(very_verbose) {
			printf("** VDC=0x%04x\n", confdesc_->vdc());
		}
		
		const config_t &conf = * config(idx);
		
		probe probe_want;
		memset(&probe_want, 0, sizeof(probe));
		probe_want.bmHint = (1); /// keep frame interval
		probe_want.bFormatIndex = conf.fmt_number;
		probe_want.bFrameIndex = conf.frm_number;
		probe_want.dwFrameInterval = HostToUSBLong( conf.interval );
		
		printf(">> interval %u\n", conf.interval);
		//set only by device: probe_want.dwMaxVideoFrameSize = HostToUSBLong( frm->dwMaxVideoFrameBufferSize );	
		/* calculate bandwidth */
#if 0
		uint32_t req_bw = frm->wWidth * frm->wHeight * fmt->bBitsPerPixel / 8;
		req_bw *= 10000000 / frame_interval + 1;
		req_bw /= 1000;
#else
		uint32_t req_bw = conf.width * conf.height * conf.bpp / 8;
		req_bw *= 10000000 / conf.interval + 1;
		req_bw /= 1000;
#endif
		//hi-speed:
		req_bw /= 8;

		//payload header
		req_bw += 12;
		if(very_verbose) {
			printf("Frame %d x %d x %d\n",conf.width,conf.height,conf.bpp);
			printf("Frame size: %d\n",conf.width * conf.height * conf.bpp / 8);
			printf("Factor: %u\n",10000000 / conf.interval + 1);
			printf("Required MaxPayloadTransferSize: %d\n", req_bw);
		}

		probe_want.dwMaxPayloadTransferSize = HostToUSBLong(req_bw);

		/* ask for it */
		uint16_t probe_size = confdesc_->vdc()<=0x100?26:34;
		uint16_t result = 0;
		uint8_t intf_num = if_stream_->get_number();
		if(very_verbose) {
			printf("++ PROBE WANT ++\n");
			dbg_print_probe(probe_want);
		}

		result = vs_control_request(SET_CUR, VS_PROBE_CONTROL, 0, intf_num, probe_size, &probe_want);
		if(result!=probe_size) {
			printf("probe SET_CUR failed\n");
			return false;
		}

		/* check, what we get */
		probe probe_get;
		memset(&probe_get, 0, sizeof(probe));
		memcpy(&probe_get, &probe_want, sizeof(probe));
		result = vs_control_request(GET_CUR, VS_PROBE_CONTROL, 0, intf_num, probe_size, &probe_get);
		if(result!=probe_size) {
			printf("probe GET_CUR failed\n");
			return false;
		}
		
		if(very_verbose) {
			printf("++ PROBE GET ++\n");
			dbg_print_probe(probe_get);
		}

		/* compare */
		// format and frame
		if(probe_want.bFormatIndex!=probe_get.bFormatIndex) {
			printf("format rejected\n");
			return false;
		}
		if(probe_want.bFrameIndex!=probe_get.bFrameIndex) {
			printf("frame rejected\n");
			return false;
		}
		if(probe_want.dwFrameInterval!=probe_get.dwFrameInterval) {
			printf("frame interval rejected\n");
			return false;
		}
		// seems ok, check b/w
		if(very_verbose) {
			if(probe_want.dwMaxPayloadTransferSize!=probe_get.dwMaxPayloadTransferSize) {
				printf("PayloadSize adjusted:\n    %lu -> %lu\n",
						USBToHostLong(probe_want.dwMaxPayloadTransferSize),
						USBToHostLong(probe_get.dwMaxPayloadTransferSize));
			}
		}
		/* sanity */
#if 0
		uint32_t should_size = frm->wWidth * frm->wHeight * fmt->bBitsPerPixel / 8;
#else
		uint32_t should_size = conf.width * conf.height * conf.bpp / 8;
#endif
		if(probe_want.dwMaxPayloadTransferSize < probe_get.dwMaxPayloadTransferSize) {
			printf("not enough bandwidth want: %u, get: %u\n",probe_want.dwMaxPayloadTransferSize,probe_get.dwMaxPayloadTransferSize);
			//return false;
		}
		if(should_size != USBToHostLong(probe_get.dwMaxVideoFrameSize)) {
			printf("wrong max video frame buffer size!\n");
			return false;
		}

		/* negotiation complete, set */
		result = vs_control_request(SET_CUR, VS_COMMIT_CONTROL, 0, intf_num, probe_size, &probe_get);
		if(result!=probe_size) {
			printf("Setting format failed\n");
			return false;
		}
		negotiated_ = true;
		memcpy(&cur_probe_, &probe_get, sizeof(probe));

		width_ = conf.width;//frm->wWidth;
		height_ = conf.height;//frm->wHeight;
		bpp_ = conf.bpp;//fmt->bBitsPerPixel;
		return true;
	}


	void camera::dbg_print_probe(probe& p) {
		printf("    [ 0] bmHint=0x%04x\n", (p.bmHint));
		printf("    [ 2] bFormatIndex=0x%02x\n", p.bFormatIndex);
		printf("    [ 3] bFrameIndex=0x%02x\n", p.bFrameIndex);
		printf("    [ 4] dwFrameInterval=%lu", USBToHostLong(p.dwFrameInterval));
		{
			/** dwFrameInterval in 100ns units:
			 * -> frames per seconds = 
			 * **/
		float fi = 10000000./float(USBToHostLong(p.dwFrameInterval));
		printf("=%0.02f fps\n", fi);
		}
		printf("    [ 8] wKeyFrameRate=%d\n", USBToHostWord(p.wKeyFrameRate));
		printf("    [10] wPFrameRate=%d\n", USBToHostWord(p.wPFrameRate));
		printf("    [12] wCompQuality=%d\n", USBToHostWord(p.wCompQuality));
		printf("    [14] wCompWindowSize=%d\n", USBToHostWord(p.wCompWindowSize));
		printf("    [16] wDelay=%d\n", USBToHostWord(p.wDelay));
		printf("    [18] dwMaxVideoFrameSize=%lu\n", USBToHostLong(p.dwMaxVideoFrameSize));
		printf("    [22] dwMaxPayloadTransferSize=%lu\n", USBToHostLong(p.dwMaxPayloadTransferSize));
	}

	void camera::begin_capture() {
		if(!negotiated_) {
			printf("set frame size etc. first!\n");
			return;
		}
		/* choose correct alternate */
		uint32_t pipe_size = USBToHostLong(cur_probe_.dwMaxPayloadTransferSize);
		uint8_t alternate = confdesc_->alternate_for_size(pipe_size);
		//printf("Choosing alternate EP: %d\n", alternate);
		if_stream_->set_alternate(alternate);

		transfer_ = new usb::isoch_transfer(if_stream_, pipe_size);
		transfer_->start();

		decoder_ = new uvc::decode_frame(this);
		decoder_->start_decode();
	}

	void camera::end_capture() {
		if(!transfer_) {
			printf("there is no capture to end\n");
			return;
		}
#if 1
		if(very_verbose) {
			printf("+ Stopping decoder\n");
		}
		decoder_->end_decode();
		delete decoder_;
		decoder_ = NULL;

		if(very_verbose) {
			printf("+ Stopping transfer\n");
		}
		transfer_->stop();
		delete transfer_;
		transfer_ = NULL;
#else
		if(very_verbose) {
			printf("+ Stopping transfer\n");
		}
		transfer_->stop();
		delete transfer_;
		transfer_ = NULL;
		
		if(very_verbose) {
			printf("+ Stopping decoder\n");
		}
		decoder_->end_decode();
		delete decoder_;
		decoder_ = NULL;
#endif
		if(very_verbose) {
			printf("+ capture ended\n");
		}
	}

	uint8_t* camera::get_frame() {
		if(decoder_) {
			return decoder_->get_frame();
		}
		return NULL;
	}
		
	float camera::fps() const {
		if(decoder_) {
			return decoder_->fps();
		}
		return 0.f;
	}
		
	uint32_t camera::kibps() const {
		if(decoder_) {
			return decoder_->kibps();
		}
		return 0;
	}
};

