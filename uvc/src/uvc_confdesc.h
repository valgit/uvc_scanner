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
#ifndef UVC_CONFDESC_H__
#define UVC_CONFDESC_H__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <vector>

#define INCLUDE_DEBUG_CODE 1

namespace uvc {

	struct video_frame_t {
		uint8_t		bFrameIndex;
		uint8_t		bmCapabilities;
		uint16_t	wWidth;
		uint16_t	wHeight;
		uint32_t	dwMinBitRate; //in bps
		uint32_t	dwMaxBitRate;
		uint32_t	dwMaxVideoFrameBufferSize;
		uint32_t	dwDefaultFrameInterval;
		uint8_t		bFrameIntervalType;
		/// continuus:
		uint32_t	dwMinFrameInterval; ///< in 100ns units
		uint32_t	dwMaxFrameInterval;
		uint32_t	dwFrameIntervalStep; ///< in amt of 100ns units
		/// discrete:
		uint32_t	*dwFrameIntervals;
#if INCLUDE_DEBUG_CODE
		void print() {
			printf("Frame #%d <%d,%d>\n", bFrameIndex, wWidth, wHeight);
			printf("  interval: %d\n", bFrameIntervalType);
			if(bFrameIntervalType) {
				for(int i=0; i<bFrameIntervalType; ++i) {
					float fps = 10000000.f/float(dwFrameIntervals[i]);
					printf("   -> %d (%f fps)\n", dwFrameIntervals[i], fps);
				}
			}
			printf("    maxVideoFrameBufferSize: %d\n", dwMaxVideoFrameBufferSize);
		}
#endif
	};

	struct video_colorformat_t {
		uint8_t		bColorPrimaries;
		uint8_t		bTransferCharacteristics;
		uint8_t		bMatrixCoefficients;
#if INCLUDE_DEBUG_CODE
		void print() {
			printf("-COLORFORAT--\n");
			printf("    bColorPrimaries=%d\n", bColorPrimaries);
			printf("    bTransferCharacteristics=%d\n", bTransferCharacteristics);
			printf("    bMatrixCoefficients=%d\n", bMatrixCoefficients);
		}
#endif
	};

	struct video_format_t {
		uint8_t		number;
		uint8_t		bmaControls; ///< 3.9.2.1
		uint8_t		bNumFrameDescriptors;
		uint8_t		bBitsPerPixel;
		/// associated frame formats
		std::vector<video_frame_t*>		frames;
		/// and color (if available)
		video_colorformat_t				*colorformat;
	};

	struct stream_ep_t {
		uint8_t		alternate;
		uint8_t		address;
		uint16_t	wMaxPacketSize;
	};

	class confdesc {
		public:
			confdesc(uint8_t* data, uint16_t length);
			~confdesc();

			inline uint16_t vdc() const { return vdc_; }
			inline uint8_t num_formats() const { return video_formats_.size(); }
			inline video_format_t* operator[](int idx) const {
				if(idx>=video_formats_.size()) {
					return NULL;
				}
				return video_formats_[idx];
			}

			uint8_t alternate_for_size(uint32_t size);

			uint8_t terminal_unit_id() const { return terminal_unit_id_; }
			uint8_t processing_unit_id() const { return processing_unit_id_; }
		private:
			/* members */
			uint16_t parse_videocontrol(uint8_t* buf, uint16_t len); 
			uint16_t parse_videostream(uint8_t* buf, uint16_t len, uint8_t alternate); 

			/* variables */
			uint8_t				*buffer_;
			uint16_t			length_;
			/* settings */
			uint16_t			vdc_;			///< video device class version (BCD)
			/* input terminal related */
			uint32_t			input_terminal_controls_;
			uint16_t			processing_unit_controls_;

			/* Unit IDs */
			uint8_t				terminal_unit_id_;
			uint8_t				processing_unit_id_;

			std::vector<video_format_t*>	video_formats_;
			std::vector<stream_ep_t*>		stream_eps_;

			// dummy
			confdesc(){};
	};
};
#endif /* UVC_CONFDESC_H__ */
