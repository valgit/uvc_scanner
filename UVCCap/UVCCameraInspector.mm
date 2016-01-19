//
//  UVCCameraInspector.mm
//  UVCCap
//
//  Created by elph on 11.11.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "UVCCameraInspector.h"
#import "UVCCamera.h"

@implementation UVCCameraInspector
@synthesize inspectorPanel;
- (void)setCamera:(UVCCamera*)cam {
	camera = cam;
	
	[autoExposureSlider setMinValue:(double)[camera minExposure]];
	[autoExposureSlider setMaxValue:(double)[camera maxExposure]];
	[autoExposureSlider setDoubleValue:(double)[camera exposure]];
	NSLog(@"exposure in [%ld,%ld]",[camera minExposure],[camera maxExposure]);
	
	if([camera autoexposure]) {
		[autoExposureButton setState:NSOnState];
		[autoExposureSlider setEnabled:FALSE];
	} else {
		[autoExposureButton setState:NSOffState];
		[autoExposureSlider setEnabled:YES];
	}
	
	if(refreshTimer) {
		[refreshTimer invalidate];
		[refreshTimer release];
	}
	
	refreshTimer = [NSTimer scheduledTimerWithTimeInterval:1.5
													target:self 
												  selector:@selector(refresh:) 
												  userInfo:nil 
												   repeats:YES];
	[refreshTimer retain];
}

- (void) refresh:(id)info {
	if(nil==camera) {
		[refreshTimer invalidate];
		[refreshTimer release];
		refreshTimer = nil;
		return;
	}
	
	if([camera autoexposure]) {
		/* [camera exposure] appears to not work correctly... */
		[autoExposureSlider setDoubleValue:(double)[camera exposure]];
	}
}

- (IBAction) autoExposureButtonAction:(id)sender {
	NSButton *btn = (NSButton*)sender;
	if([btn state]==NSOnState) {
		[camera setAutoexposure:YES];
		[autoExposureSlider setEnabled:NO];
	} else {
		[camera setAutoexposure:NO];
		[autoExposureSlider setEnabled:YES];
	}
}
- (IBAction) autoExposureSliderAction:(id)sender {
	NSLog(@"Exposure ****");
	NSSlider *sld = (NSSlider*)sender;
	long lv = (long)[sld doubleValue];
	NSLog(@"Exposure SET-->%ld",lv);
	[camera setExposure:lv];
}
@end
