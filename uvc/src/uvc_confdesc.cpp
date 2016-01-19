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
#include "uvc_confdesc.h"
#include "uvc_types.h"

namespace { /* anonymous, local and debug */
	bool very_verbose = false;
};

namespace uvc {
	confdesc::confdesc(uint8_t* data, uint16_t length)
	: buffer_(data), length_(length)
	{
		/* init stuff */
		vdc_ = 0;

		/* find, where interface descriptor definition starts */
		uint8_t*	buf = buffer_;
		uint16_t	len = length_;
		/* go over everything we might find */
		while(len>2) {
			/* find next INTERFACE_CLASS_VIDEO */
			while(len>2 && !(buf[1]==0x04 && buf[5]==INTERFACE_CLASS_VIDEO)) {
				len -= buf[0];
				buf += buf[0];
			}
			/* already at end? */
			if(!len) {
				break;
			}
			if(very_verbose) {
				printf("*** VIDEO interface\n");
			}
			uint8_t intf_num = buf[2];
			uint8_t alternate = buf[3];
			uint16_t skip = 0;
			switch(buf[6]) { ///< sub-class
				case SC_VIDEOCONTROL:
					skip = parse_videocontrol(buf, len);
					break;
				case SC_VIDEOSTREAMING:
					skip = parse_videostream(buf, len, alternate);
					break;
				default:
					printf("unexpected sub-class\n");
					break;
			}

			if(skip) {
				len -= skip;
				buf += skip;
			} else {
				len -= buf[0];
				buf += buf[0];
			}
		}
	}

	confdesc::~confdesc() {
		/// note: buffer_ should be handles by IOKit
	}

	uint16_t confdesc::parse_videocontrol(uint8_t* buf, uint16_t len) {
		uint16_t start_len = len;
		if(very_verbose) {
			printf("processing video control...\n");
		}
		/* skip interface header */
		len -= buf[0];
		buf += buf[0];

		while(len>2) {
			/* stop, if we hit the next interface */
			if(buf[1]==USB_INTERFACE) {
				break;
			}
			/* check, what we've got here */
			if(very_verbose) {
				printf("* got 0x%02x/0x%02x\n", buf[1], buf[2]);
			}
			switch(buf[1]) {
				case CS_INTERFACE:
						switch(buf[2]) {
							case VC_HEADER:
								{
									vdc_ = USBToHostWord(*(uint16_t*)&buf[3]);
									uint32_t dwClockFrequency = USBToHostLong(*(uint32_t*)&buf[7]);
									if(very_verbose) {
										printf("VC Header: 0x%04x, %d\n", vdc_, dwClockFrequency);
									}
								}
								break;
							case VC_INPUT_TERMINAL:
								{
									//uint8_t bTerminalID = buf[3];
									//printf(">> INPUT_TERMINAL_ID: %d\n", bTerminalID);
									terminal_unit_id_ = buf[3];

									input_terminal_controls_ = *((uint32_t*)&buf[15]);
									if(very_verbose) { //debug
										uint32_t fields = (input_terminal_controls_);
										printf("-----------------------------------------------\n");
										printf("-- INPUT TERMINAL -----------------------------\n");
										printf("-----------------------------------------------\n");
										printf("Scanning Mode:\t%s\n", fields&(1<< 0)?"yes":"no");
										printf("Auto-Exposure Mode:\t%s\n", fields&(1<< 1)?"yes":"no");
										printf("Auto-Exposure Priority:\t%s\n", fields&(1<< 2)?"yes":"no");
										printf("Exposure Time (Absolute):\t%s\n", fields&(1<< 3)?"yes":"no");
										printf("Exposure Time (Relative):\t%s\n", fields&(1<< 4)?"yes":"no");
										printf("Focus (Absolute):\t%s\n", fields&(1<< 5)?"yes":"no");
										printf("Focus (Relative):\t%s\n", fields&(1<< 6)?"yes":"no");
										printf("Iris (Absolute):\t%s\n", fields&(1<< 7)?"yes":"no");
										printf("Iris (Relative):\t%s\n", fields&(1<< 8)?"yes":"no");
										printf("Zoom (Absolute):\t%s\n", fields&(1<< 9)?"yes":"no");
										printf("Zoom (Relative):\t%s\n", fields&(1<<10)?"yes":"no");
										printf("PanTilt (Absolute):\t%s\n", fields&(1<<11)?"yes":"no");
										printf("PanTilt (Relative):\t%s\n", fields&(1<<12)?"yes":"no");
										printf("Roll (Absolute):\t%s\n", fields&(1<<13)?"yes":"no");
										printf("Roll (Relative):\t%s\n", fields&(1<<14)?"yes":"no");
										printf("Focus, Auto:\t%s\n", fields&(1<<17)?"yes":"no");
										printf("Privacy:\t%s\n", fields&(1<<18)?"yes":"no");
									}
								}
								break;
							case VC_OUTPUT_TERMINAL:
								break;
							case VC_SELECTOR_UNIT:
								break;
							case VC_PROCESSING_UNIT:
								{
									//uint8_t bUnitID = buf[3];
									//printf(">> PROCESSING UNIT ID: %d\n", bUnitID);
									processing_unit_id_ = buf[3];
									processing_unit_controls_ = *((uint16_t*)&buf[8]);
									if(very_verbose) { //debug
										uint16_t fields = ( processing_unit_controls_ );
										printf("-----------------------------------------------\n");
										printf("-- PROCESSING UNIT ----------------------------\n");
										printf("-----------------------------------------------\n");
										printf("SIZE: %d\n", buf[7]); /// BUG here?
										printf("Brightness:\t%s\n", fields&(1<< 0)?"yes":"no");
										printf("Contrast:\t%s\n", fields&(1<< 1)?"yes":"no");
										printf("Hue:\t%s\n", fields&(1<< 2)?"yes":"no");
										printf("Saturation:\t%s\n", fields&(1<< 3)?"yes":"no");
										printf("Sharpness:\t%s\n", fields&(1<< 4)?"yes":"no");
										printf("Gamma:\t%s\n", fields&(1<< 5)?"yes":"no");
										printf("White Balance Temperature:\t%s\n", fields&(1<< 6)?"yes":"no");
										printf("White Balance Component:\t%s\n", fields&(1<< 7)?"yes":"no");
										printf("Backlight Compensation:\t%s\n", fields&(1<< 8)?"yes":"no");
										printf("Gain:\t%s\n", fields&(1<< 9)?"yes":"no");
										printf("Power Line Frequency:\t%s\n", fields&(1<<10)?"yes":"no");
										for(int i=11; i<16; ++i)
											printf("Reserved(%d) is zero:\t%s\n", i, (fields&(1<<i))==0?"yes":"no");
									}
								}
								break;
							case VC_EXTENSION_UNIT:
								break;

						}
					/* end case CS_INTERFACE */
					break;
				case USB_ENDPOINT:
					{
						//printf("* EP\n");
					}
					break;
				case CS_ENDPOINT:
					if(buf[2]==EP_INTERRUPT) {
						uint16_t wMaxTransferSize = USBToHostWord(*(uint16_t*)&buf[3]);
						//printf("VC, CS_ENDPOINT/INTERRUPT %d\n",wMaxTransferSize);
						break;
					}
					break;
			}

			len -= buf[0];
			buf += buf[0];
		}
		return start_len-len;
	}


	uint16_t confdesc::parse_videostream(uint8_t* buf, uint16_t len, uint8_t alternate) {
		uint16_t start_len = len;
		if(very_verbose) {
			printf("processing video stream...\n");
		}
		/* skip interface header */
		len -= buf[0];
		buf += buf[0];

		while(len>2) {
			/* stop, if we hit the next interface */
			if(buf[1]==USB_INTERFACE) {
				break;
			}
			/* check, what we've got here */
			if(very_verbose) {
				printf("* got 0x%02x/0x%02x\n", buf[1], buf[2]);
			}
			switch(buf[1]) {
				case CS_INTERFACE:
						switch(buf[2]) {
							case VS_INPUT_HEADER:
								{
									uint8_t bNumFormats = buf[3];
									uint8_t bEndpointAddress = buf[6];
									//printf("INPUT HEADER ep->0x%02x\n", bEndpointAddress);

									uint8_t bTriggerSupport = buf[10];
									uint8_t bTriggerUsage = buf[11];
									//printf("TS: %d, TU: %d\n", bTriggerSupport, bTriggerUsage);

									uint8_t bControlSize = buf[12];

									/* this header/descriptor should appear before
									 * any other; therefore, video_fomats_ should
									 * be still empty. but rather check! */
									if(!video_formats_.empty()) {
										printf("ARGH! This should not happen\n");
									}
									for(int i=0; i<bNumFormats; ++i) {
										video_format_t *fmt = new video_format_t;
										fmt->number = i+1;
										//we care just for the least significant byte
										//(actually, just bits 0-5 are defined by
										//std
										fmt->bmaControls = buf[13+i*bControlSize];
										video_formats_.push_back(fmt);
									}
								}
								break;
							case VS_FORMAT_UNCOMPRESSED:
								{
									/* we should have video_formats_ by now... */
									if(video_formats_.empty()) {
										printf("ARGH! SHOULD not happen\n");
										break;
									}
									uint8_t bFormatIndex = buf[3];
									if(bFormatIndex > video_formats_.size()+1) {
										printf("Unexpected format!\n");
										break;
									}
									video_format_t *fmt = video_formats_[bFormatIndex-1];
									fmt->bNumFrameDescriptors = buf[3];
									fmt->bBitsPerPixel = buf[21];

								}
								break;
							case VS_FRAME_UNCOMPRESSED:
								{
									video_frame_t *frm = new video_frame_t;
									frm->bFrameIndex = buf[3];
									frm->bmCapabilities = buf[4];
									frm->wWidth = USBToHostWord(*(uint16_t*)&buf[5]);
									frm->wHeight = USBToHostWord(*(uint16_t*)&buf[7]);

									frm->dwMinBitRate = USBToHostLong(*(uint32_t*)&buf[9]);
									frm->dwMaxBitRate = USBToHostLong(*(uint32_t*)&buf[13]);

									frm->dwMaxVideoFrameBufferSize = USBToHostLong(*(uint32_t*)&buf[17]);

									frm->dwDefaultFrameInterval = USBToHostLong(*(uint32_t*)&buf[21]);

									frm->bFrameIntervalType = buf[25];
									if(frm->bFrameIntervalType==0) {
										// continuus
										frm->dwMinFrameInterval = USBToHostLong(*(uint32_t*)&buf[26]);
										frm->dwMaxFrameInterval = USBToHostLong(*(uint32_t*)&buf[30]);
										frm->dwFrameIntervalStep = USBToHostLong(*(uint32_t*)&buf[34]);
										frm->dwFrameIntervals = NULL;
									} else {
										// discreete
										frm->dwFrameIntervals = new uint32_t[frm->bFrameIntervalType];
										for(int i=0; i<frm->bFrameIntervalType; ++i) {
											frm->dwFrameIntervals[i] = USBToHostLong(*(uint32_t*)&buf[26+4*i]);
										}
									}
									
									if(very_verbose) {
										frm->print();
									}
									//sanity
									if(video_formats_.empty()) {
										throw "Out of order video frames";
									}
									video_format_t *vfmt = video_formats_.back();
									vfmt->frames.push_back(frm);
								}
								break;
							case VS_COLORFORMAT:
								{
									video_colorformat_t *fmt = new video_colorformat_t;
									fmt->bColorPrimaries = buf[3];
									fmt->bTransferCharacteristics = buf[4];
									fmt->bMatrixCoefficients = buf[5];
								
									if(very_verbose) {	
										fmt->print();
									}

									if(video_formats_.empty()) {
										throw "Out of order video frames";
									}
									video_format_t *vfmt = video_formats_.back();
									vfmt->colorformat = fmt;
								}
								break;

						}
					/* end case CS_INTERFACE */
					break;
				case USB_ENDPOINT:
					{
						uint8_t address = buf[2];
						uint8_t bmAttributes = buf[3];
						uint16_t wMaxPacketSize = USBToHostWord(*((uint16_t*)(&buf[4])));
						/* check, if this one is useful to us */
						uint8_t direction_in = address>>7; ///< 0=Out, 1=In
						uint8_t is_isoch = (bmAttributes&3)==1;
						if(direction_in && is_isoch) {
							if(very_verbose) {
								printf("* Useful EP @0x%02x (alt=%d), rate=%d\n", address, alternate, wMaxPacketSize);
							}
							stream_ep_t *ep = new stream_ep_t;
							ep->alternate = alternate;
							ep->address = address;
							ep->wMaxPacketSize = wMaxPacketSize;
							stream_eps_.push_back(ep);
						}
					}
					break;
				//case CS_ENDPOINT:
					//break;
			}

			len -= buf[0];
			buf += buf[0];
		}
		return start_len-len;
	}

	uint8_t confdesc::alternate_for_size(uint32_t size) {
		for(std::vector<stream_ep_t*>::iterator i=stream_eps_.begin();
				i!=stream_eps_.end(); ++i) {
			if( (*i)->wMaxPacketSize > size + 64 ) {
				return (*i)->alternate;
			}
		}
		return 255; // ouch TODO raise something
	}
};

