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
#import "CapView.h"
#import <stdexcept>
#import <uvc/uvc_camera.h>
#import <uvc/uvc_colorcvt.h>

@implementation CapView

- (id)initWithFrame:(NSRect)frame {
    [super initWithFrame:frame];
	/* initialize my stuff*/
	NSLog(@"CapView initializing...");
	cam_ = NULL;
	frame_img_ = nil;
	try {
		/* init camera instance */
		cam_ = new uvc::camera(0);
		/* configure camera */
		cam_->ctl().auto_exposure(false);
		cam_->ctl().exposure(8);
		
		{
			cam_->ctl().exposure_minmax(exposure_min_, exposure_max_);
		}
		
		
		/* set up streaming */
		if(!cam_->set_config(640, 480, 16, 30)) {
			NSLog(@"Coult not config camera!");
			delete cam_;
			cam_ = NULL;
			return nil;
		}
		
		framestore_ = new uint8_t[640*480*3];
		
		cam_->begin_capture();
		cap_timer_ = [NSTimer scheduledTimerWithTimeInterval:0.001 target:self selector:@selector(fetchFrame:) userInfo:nil repeats:YES];
		[cap_timer_ retain];
	} catch(std::exception& exc) {
		NSLog(@"Failed: %s", exc.what());
		return nil;
	}
	NSLog(@"CapView initializing successfull");
	return self;
}

- (void)dealloc {
	/* clean-up my stuff */
	if(cam_) {
		cam_->end_capture();
		delete cam_;
		delete framestore_;
	}
	[cap_timer_ invalidate];
	[cap_timer_ release];
	if(frame_img_) {
		[frame_img_ release];
	}
	/* super */
    [super dealloc];
}

- (void)drawRect:(NSRect)rect {
	[[NSColor redColor] set];
    NSRectFill([self bounds]);
	
	if(frame_img_) {
		NSRect imageRect = NSMakeRect(0,0,0,0);
		imageRect.size = [frame_img_ size];
		[frame_img_ drawInRect:[self bounds] fromRect:imageRect operation:NSCompositeSourceOver fraction:1.0];
	}
}

- (BOOL)isOpaque {
    return YES;
}

-(void)fetchFrame:(NSTimer*)timer {
	uint8_t *frame = cam_->get_frame();
	if(!frame) {
		// no frame, skip
		return;
	}
	/* convert to NSImage */
	uvc::yuv422_to_rgb(framestore_, frame, 640, 480, 640*3);
	uint8_t *planes[] = {framestore_};
	NSBitmapImageRep *bir =
	[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:planes
											pixelsWide:640
											pixelsHigh:480
										 bitsPerSample:8
									   samplesPerPixel:3 
											  hasAlpha:NO 
											  isPlanar:NO 
										colorSpaceName:NSDeviceRGBColorSpace
										   bytesPerRow:640*3 bitsPerPixel:24];
	
	if(frame_img_) {
		[frame_img_ release];
	}
	frame_img_ = [[NSImage alloc] initWithData:[bir TIFFRepresentation]];
	[frame_img_ retain];
	[bir release];
	
	delete [] frame;
	[self setNeedsDisplay:YES];
}

- (void)awakeFromNib {
	[exposure_slider_ setMinValue:exposure_min_];
	[exposure_slider_ setMaxValue:exposure_max_];
}

- (IBAction) slider_exposure:(id)sender {
	int val = [exposure_slider_ intValue];
	if(cam_) {
		int nval = cam_->ctl().exposure(val);
		NSLog(@"nval: %d", nval);
	}
}

@end
