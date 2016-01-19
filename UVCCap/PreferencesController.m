//
//  PreferencesController.m
//  UVCCap
//
//  Created by valery brasseur on 23/12/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#import "PreferencesController.h"

@implementation PreferencesController


- (id) init {
    if(self = [super initWithWindowNibName:@"Preferences"]) {
        prefs = [[NSUserDefaults standardUserDefaults] retain];
        //[self.fileTypes setAction:@selector(fileTypeChanged:)];
    }
    return self;	
}

//TODO: [standardUserDefaults setObject:[mOutQuality stringValue] forKey:@"outputQuality"];
/*
 
 // saving an NSInteger
 [standardUserDefaults setInteger:42 forKey:@"integer"];
 
 // saving a Float
 [standardUserDefaults setFloat:3.1415 forKey:@"float"];

 NSString *temp;
 temp = [standardUserDefaults objectForKey:@"outputQuality"];
 if (temp != nil)
 [mOutQuality setStringValue:temp];

 // getting an Float object
 float myFloat = [standardUserDefaults floatForKey:@"floatKey"]
 */


- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (void)dealloc
{
    // use bindings ?
    //[prefs synchronize];
    
    // update our prefs...
    //[prefs setInteger:[mQuality intValue] forKey:@"outputQuality"];
    //[prefs release];
    [super dealloc];
}

#pragma mark -
#pragma mark handle change

- (IBAction)chooseDestDir: (id)sender
{
	NSOpenPanel* openPanel;
	
	openPanel = [[NSOpenPanel openPanel] retain];
	[openPanel setCanChooseFiles: NO];
	[openPanel setTitle: @"Choose Destination Directory"];
	[openPanel setCanChooseDirectories: YES];
	[openPanel setAllowsMultipleSelection: NO];
    
	if ([openPanel runModal] == NSOKButton) {
		NSString* dirname;
        
		dirname = [[openPanel URL]  path];
        // TODO: check effectiveness...
        //[mSetPath setStringValue:dirname];
          
        // use of binding ?
		//[prefs setObject:dirname forKey:@"outputDirectory"];
		//[prefs synchronize];
		NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
		[defaults setValue:dirname forKeyPath:@"values.outputDirectory"];
        
        
	}
    [openPanel release];
}

@end
