/* ******************************************************************
 
 Copyright (c) 2010 max.grosse < max . grosse {at} ioctl . eu >
 
 This  software is  provided 'as-is', without  any express or  implied
 warranty.  In  no event  will  the  authors  be  held liable for  any
 damages arising from the use of this software.
 
 Permission  is  granted  to  anyone  to  use  this  software for  any
 purpose,  including  commercial applications, and  to  alter  it  and
 redistribute  it  freely,  subject  to  the  following  restrictions:
 
 1. The  origin  of  this  software  must not  be  misrepresented;
 you  must not claim that you wrote the original software.  If you
 use this software in a product,  an acknowledgment in the product
 documentation would be appreciated but is not required.
 
 2. Altered source  versions must be  plainly marked as such,  and
 must not be misrepresented as being the original software.
 
 3. This notice  may not  be removed or  altered  from  any source
 distribution.
 
 ****************************************************************** */
#import "UVCCamera.h"
#import <uvc/uvc_colorcvt.h>

@implementation UVCCamera
- (id) initWithVendor:(NSUInteger)vend product:(NSUInteger)prod {
	self = [super init];
	if(self) {
		camera = new uvc::camera(vend, prod);
	}
	return self;
}
- (void) dealloc {
	//NSLog(@"...dealloc camera");
	if(framestore) {
		delete [] framestore;
	}
	if(camera) {
		delete camera;
		camera = NULL;
	}
	[super dealloc];
}

- (unsigned) width { return camera->width(); }
- (unsigned) height { return camera->height(); }
- (unsigned) bpp { return camera->bpp(); }

- (unsigned) configCount { return camera->num_configs(); }

- (BOOL) setConfig:(unsigned)idx {
	NSAssert(NULL!=camera,@"Camera not ready");
	
	//NSLog(@"Setting config..");
	BOOL res = camera->set_config(idx);
	//NSLog(@"Config set: %@",res?@"GOOD":@"BAD");
	
	if(framestore) {
		delete [] framestore;
	}
	framestore = new uint8_t[camera->width()*camera->height()*3];
	
	return res;
}

- (void) beginCapture { 
	NSAssert(NULL!=camera,@"Camera not ready");
	camera->begin_capture();
}
- (void) endCapture {
	NSAssert(NULL!=camera,@"Camera not ready");
	//NSLog(@"Stopping capture");
	camera->end_capture();
	//NSLog(@"Stopping capture: STOPPED");
}

- (NSImage*) grabFrame {
	NSAssert(NULL!=camera,@"Camera not ready");
	//NSLog(@"Grabbing.. %d x %d", camera->width(), camera->height());
	
	uint8_t *frame = camera->get_frame();
	if(!frame) {
		// no frame, skip
		return nil;
	}
	
	/* convert to NSImage */
	uvc::yuv422_to_rgb(framestore, frame, camera->width(), camera->height(), camera->width()*3);
	uint8_t *planes[] = {framestore};
	NSBitmapImageRep *bir = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:planes
																	pixelsWide:camera->width()
																	pixelsHigh:camera->height()
																 bitsPerSample:8
															   samplesPerPixel:3 
																	  hasAlpha:NO 
																	  isPlanar:NO 
																colorSpaceName:NSDeviceRGBColorSpace
																   bytesPerRow:camera->width()*3
																  bitsPerPixel:24];
	
	NSImage *frameImage = [[NSImage alloc] initWithData:[bir TIFFRepresentation]];
	[bir release];
	
	delete [] frame;
	return [frameImage autorelease];
}

- (float) fps { return camera->fps(); }
- (uint32_t) kibps { return camera->kibps(); }

- (NSArray*) configStrings {
	NSMutableArray *ca = [NSMutableArray arrayWithCapacity:[self configCount]];
	const uvc::camera::configs_t &cfgs = * (camera->configurations());
	uvc::camera::configs_t::const_iterator iter = cfgs.begin();
	for(; iter!=cfgs.end(); ++iter) {
		uvc::camera::config_t *cf = *iter;
		NSString *s = [NSString stringWithFormat:@"%dx%dx%d %.2ffps",
					  cf->width,
					  cf->height,
					  cf->bpp,
					  cf->interval_fps];
		[ca addObject:s];
	}
	return ca;
}
- (BOOL) autoexposure {
	return camera->ctl().auto_exposure();
}
- (long) minExposure {
	long emin, emax;
	camera->ctl().exposure_minmax(emin, emax);
	return emin;
}
- (long) maxExposure {
	long emin, emax;
	camera->ctl().exposure_minmax(emin, emax);
	return emax;
}
- (long) exposure {
	return camera->ctl().exposure();
}

- (void) setAutoexposure:(BOOL)yes {
	camera->ctl().auto_exposure(yes);
}

- (void) setExposure:(long)value {
	camera->ctl().exposure(value);
}

@end
