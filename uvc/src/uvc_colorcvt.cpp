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
#include "uvc_colorcvt.h"
#include <stdint.h>
#include <algorithm>
namespace uvc {
	inline uint8_t clamp255(float f) {
		f *= 255.f;
		if(f>255.f) return 255;
		if(f<=0.f) return 0;
		return uint8_t(f);
	}
	inline uint8_t clip(int32_t c) {
		if(c<0) return 0;
		if(c>255) return 255;
		return uint8_t(c);
		//return c;
	}

	void yuv422_to_bgr(void* ptr_bgr, void* ptr_yuv, int width, int height, int rgb_rowbytes) {
		uint8_t *yuv = (uint8_t*)ptr_yuv;
		uint8_t *bgr = (uint8_t*)ptr_bgr;

		//const uint8_t BYTE_U0 = 1;
		//const uint8_t BYTE_Y0 = 2;
		//const uint8_t BYTE_V0 = 3;
		//const uint8_t BYTE_Y1 = 0;

		const uint8_t BYTE_U0 = 1;
		const uint8_t BYTE_Y0 = 0;
		const uint8_t BYTE_V0 = 3;
		const uint8_t BYTE_Y1 = 2;

		uint32_t steps = (width>>1)*height;
		int pix_w = 0;
		while(steps--) {
			const uint8_t u = yuv[BYTE_U0];
			const uint8_t v = yuv[BYTE_V0];
			const uint8_t y0 = yuv[BYTE_Y0];
			const uint8_t y1 = yuv[BYTE_Y1];
			yuv += 4;

			const int32_t C0 = y0 - 16;
			const int32_t C1 = y1 - 16;
			const int32_t D0 = u - 128;
			const int32_t E0 = v - 128;
			
			
			uint32_t tr0 = (298*C0 + 409*E0 + 128)>>8;
			uint32_t tg0 = (298*C0 - 100*D0 - 208*E0 + 128)>>8;
			uint32_t tb0 = (298*C0 + 516*D0 + 128)>>8;
			uint32_t tr1 = (298*C1 + 409*E0 + 128)>>8;
			uint32_t tg1 = (298*C1 - 100*D0 - 208*E0 + 128)>>8;
			uint32_t tb1 = (298*C1 + 516*D0 + 128)>>8;
			
			uint8_t r0 = clip(tr0);
			uint8_t g0 = clip(tg0);
			uint8_t b0 = clip(tb0);	
			uint8_t r1 = clip(tr1);
			uint8_t g1 = clip(tg1);
			uint8_t b1 = clip(tb1);	

			*(bgr++) = b0;
			*(bgr++) = g0;
			*(bgr++) = r0;
			*(bgr++) = b1;
			*(bgr++) = g1;
			*(bgr++) = r1;
			pix_w += 2;
			if(pix_w == width) {
				int bytes_pad = rgb_rowbytes - pix_w*3;
				bgr += bytes_pad;
				pix_w = 0;
			}
		}
	}
	
	void yuv422_to_planes(void* ptr_yuv, int width, int height, uint8_t** planes) {
		uint8_t *yuv = (uint8_t*)ptr_yuv;
		
		uint8_t *red = planes[0];
		uint8_t *green = planes[1];
		uint8_t *blue = planes[2];
		
		const uint8_t BYTE_U0 = 1;
		const uint8_t BYTE_Y0 = 0;
		const uint8_t BYTE_V0 = 3;
		const uint8_t BYTE_Y1 = 2;
		
		uint32_t steps = (width>>1)*height;
		//int pix_w = 0;
		while(steps--) {
			const uint8_t u = yuv[BYTE_U0];
			const uint8_t v = yuv[BYTE_V0];
			const uint8_t y0 = yuv[BYTE_Y0];
			const uint8_t y1 = yuv[BYTE_Y1];
			yuv += 4;
			
			const int32_t C0 = y0 - 16;
			const int32_t C1 = y1 - 16;
			const int32_t D0 = u - 128;
			const int32_t E0 = v - 128;
			
			const uint8_t r0 = clip((298*C0 + 409*E0 + 128)>>8);
			const uint8_t g0 = clip((298*C0 - 100*D0 - 208*E0 + 128)>>8);
			const uint8_t b0 = clip((298*C0 + 516*D0 + 128)>>8);	
			const uint8_t r1 = clip((298*C1 + 409*E0 + 128)>>8);
			const uint8_t g1 = clip((298*C1 - 100*D0 - 208*E0 + 128)>>8);
			const uint8_t b1 = clip((298*C1 + 516*D0 + 128)>>8);	
			*(red++) = r0;
			*(red++) = r1;
			*(green++) = g0;
			*(green++) = g1;
			*(blue++) = b0;
			*(blue++) = b1;
		}
	}
	
	void yuv422_to_rgb(void* ptr_rgb, void* ptr_yuv, int width, int height, int rgb_rowbytes) {
		uint8_t *yuv = (uint8_t*)ptr_yuv;
		uint8_t *rgb = (uint8_t*)ptr_rgb;
		
		//const uint8_t BYTE_U0 = 1;
		//const uint8_t BYTE_Y0 = 2;
		//const uint8_t BYTE_V0 = 3;
		//const uint8_t BYTE_Y1 = 0;
		
		const uint8_t BYTE_U0 = 1;
		const uint8_t BYTE_Y0 = 0;
		const uint8_t BYTE_V0 = 3;
		const uint8_t BYTE_Y1 = 2;
		
		uint32_t steps = (width>>1)*height;
		int pix_w = 0;
		while(steps--) {
			const uint8_t u = yuv[BYTE_U0];
			const uint8_t v = yuv[BYTE_V0];
			const uint8_t y0 = yuv[BYTE_Y0];
			const uint8_t y1 = yuv[BYTE_Y1];
			yuv += 4;
			
			const int32_t C0 = y0 - 16;
			const int32_t C1 = y1 - 16;
			const int32_t D0 = u - 128;
			const int32_t E0 = v - 128;
#if 0		
			const uint8_t r0 = clip((298*C0 + 409*E0 + 128)>>8);
			const uint8_t g0 = clip((298*C0 - 100*D0 - 208*E0 + 128)>>8);
			const uint8_t b0 = clip((298*C0 + 516*D0 + 128)>>8);	
			const uint8_t r1 = clip((298*C1 + 409*E0 + 128)>>8);
			const uint8_t g1 = clip((298*C1 - 100*D0 - 208*E0 + 128)>>8);
			const uint8_t b1 = clip((298*C1 + 516*D0 + 128)>>8);	
#else
			uint32_t tr0 = (298*C0 + 409*E0 + 128)>>8;
			uint32_t tg0 = (298*C0 - 100*D0 - 208*E0 + 128)>>8;
			uint32_t tb0 = (298*C0 + 516*D0 + 128)>>8;
			uint32_t tr1 = (298*C1 + 409*E0 + 128)>>8;
			uint32_t tg1 = (298*C1 - 100*D0 - 208*E0 + 128)>>8;
			uint32_t tb1 = (298*C1 + 516*D0 + 128)>>8;
			
			uint8_t r0 = clip(tr0);
			uint8_t g0 = clip(tg0);
			uint8_t b0 = clip(tb0);	
			uint8_t r1 = clip(tr1);
			uint8_t g1 = clip(tg1);
			uint8_t b1 = clip(tb1);	
#endif
			*(rgb++) = r0;
			*(rgb++) = g0;
			*(rgb++) = b0;
			*(rgb++) = r1;
			*(rgb++) = g1;
			*(rgb++) = b1;
			pix_w += 2;
			if(pix_w == width) {
				int bytes_pad = rgb_rowbytes - pix_w*3;
				rgb += bytes_pad;
				pix_w = 0;
			}
		}
	}
};

