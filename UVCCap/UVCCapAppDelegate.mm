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

#import "UVCCapAppDelegate.h"
#import "UVCProbeWindowController.h"
#import "UVCCameraWindowController.h"
#import "UVCCameraInspector.h"

@implementation UVCCapAppDelegate
@synthesize probeWindowController;
@synthesize cameraInspector;

- (void)nullThread:(id)obj {
#pragma unused(obj)
}

+ (void)initialize {
    NSString *outputDirectory = NSHomeDirectory();
    outputDirectory = [outputDirectory stringByAppendingPathComponent:@"Pictures"];
    
    NSDictionary *defaults = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"imgNum", [NSNumber numberWithInt:0], 
                              @"outputQuality", [NSNumber numberWithFloat:100.0], 
                              @"outputDirectory",outputDirectory,nil];
    
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    // if your application supports resetting a subset of the defaults to
    // factory values, you should set those values
    // in the shared user defaults controller
    
    // Set the initial values in the shared user defaults controller
    [[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:defaults];
    NSLog(@"defaults register");
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	/* tell cocoa we are multithreaded */
	if(NO==[NSThread isMultiThreaded]) {
		NSLog(@"..setting multithreaded..");
		[NSThread detachNewThreadSelector:@selector(nullThread:) toTarget:self withObject:nil];
	}
	
	self.probeWindowController = [[UVCProbeWindowController alloc] initWithWindowNibName:@"ProbeWindow"];
	[probeWindowController showWindow:self];
	
	self.cameraInspector = [[UVCCameraInspector alloc] initWithWindowNibName:@"CameraInspector"];
	[cameraInspector showWindow:self];
    
   }

- (IBAction) showCameraInspector:(id)sender {
	[cameraInspector.inspectorPanel orderFront:sender];
}
@end
