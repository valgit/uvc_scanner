//
//  UVCCameraInspector.h
//  UVCCap
//
//  Created by elph on 11.11.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class UVCCamera;

@interface UVCCameraInspector : NSWindowController {
	IBOutlet NSPanel						*inspectorPanel;
	UVCCamera								*camera;
	
	IBOutlet NSButton						*autoExposureButton;
	IBOutlet NSSlider						*autoExposureSlider;
	
	NSTimer									*refreshTimer;
}
@property (assign)	 NSPanel				*inspectorPanel;
- (void)setCamera:(UVCCamera*)cam;

- (IBAction) autoExposureButtonAction:(id)sender;
- (IBAction) autoExposureSliderAction:(id)sender;
@end
