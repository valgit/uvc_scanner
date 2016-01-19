//
//  PreferencesController.h
//  UVCCap
//
//  Created by valery brasseur on 23/12/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface PreferencesController : NSWindowController
{
    IBOutlet NSTextField * mSetPath;
    IBOutlet NSButton *mBrowseDir;
    IBOutlet NSSlider *mQuality;
    IBOutlet NSPopUpButton *mFormat;
    
@private
	NSUserDefaults *prefs;
}

- (IBAction)chooseDestDir: (id)sender;

@end
