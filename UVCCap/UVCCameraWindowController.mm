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
#import "UVCCameraWindowController.h"
#import "UVCCamera.h"
#import "UVCCameraView.h"
#import "UVCCapAppDelegate.h"

#define OUT_DIR @"outputDirectory"

@implementation UVCCameraWindowController
@synthesize camera;

#pragma mark -

- (id) initWithWindowNibName:(NSString*)name Camera:(UVCCamera*)cam {
	self = [super initWithWindowNibName:name];
	if(self) {
		self.camera = cam;
        
 
	}
	return self;
}

- (void) windowDidLoad {
	//cameraView.camera = camera;
	//[cameraView beginStreaming];
	if(nil==camera) {
		NSLog(@"not goo, no camera!");
		return;
	}
	
	refreshTimer = [NSTimer scheduledTimerWithTimeInterval:0.01
													target:self 
												  selector:@selector(refresh:) 
												  userInfo:nil 
												   repeats:YES];
	[refreshTimer retain];
	
	NSArray *configs = [camera configStrings];
	[configSelectButton removeAllItems];
	[configSelectButton addItemsWithTitles:configs];
	
	if(NO==[camera setConfig:0]) {
		NSLog(@"Could not configure camera!");
		return;
	}
	NSLog(@"Camera configured for %dx%d",[camera width],[camera height]);
	// start capturing
	[camera beginCapture];
	
	//[camera beginCapture];
	//[camera endCapture];
}
- (IBAction) configSelectionChange:(id)sender {
	NSLog(@"Selected: %ld",[sender indexOfSelectedItem]);
	//[cameraView switchConfig:[sender indexOfSelectedItem]];
	
	[refreshTimer invalidate];
	[refreshTimer release];
	
	[camera endCapture];
	[camera setConfig:[sender indexOfSelectedItem]];
	[camera beginCapture];
	
	refreshTimer = [NSTimer scheduledTimerWithTimeInterval:0.001
													target:self 
												  selector:@selector(refresh:) 
												  userInfo:nil 
												   repeats:YES];
	[refreshTimer retain];
}

- (IBAction) grabShot:(id)sender {
    NSLog(@"grabShot()");
}

- (void) dealloc {
	NSLog(@"WC->dealloc");
	//[super dealloc];
	
	//[camera endCapture];
	
	//[camera release];
	//camera = nil;
	
	//[cameraView endStreaming];
	//[camera release];
	//camera = nil;
	//
	[super dealloc];
}

#pragma mark Window-Delegates
- (void)windowWillClose:(NSNotification *)notification {
	/*[camera endCapture];
	[camera disconnect];
	[camera release];*/
	NSLog(@"Closing.. retaincount: %lu",[self retainCount]);

	[refreshTimer invalidate];
	[refreshTimer release];
#if 1
	[camera endCapture];
	[camera release];
	camera = nil;
#endif
	[self release];
}

- (void) refresh:(id)ob {
	if(nil==camera) {
		return;
	}
	
	NSImage *img = [camera grabFrame];
	if(nil==img) {
		/* frame not updated yet, thus
		 simply return */
		return;
	}
	[cameraView.cameraImage release];
	cameraView.cameraImage = [img retain];
	
	[fpsTF setStringValue:[NSString stringWithFormat:@"%.2f fps",[camera fps]]];
	[kbpsTF setStringValue:[NSString stringWithFormat:@"%u kibps",[camera kibps]]];
	
	[cameraView setNeedsDisplay:YES];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
	UVCCapAppDelegate *ad = (UVCCapAppDelegate*)[NSApp delegate];
	[ad.cameraInspector setCamera:camera];
}

# pragma mark First Responder Stuff

- (void)copy:(id)sender {
	if(nil!=cameraView.cameraImage) {
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard clearContents];
        NSArray *copiedObjects = [NSArray arrayWithObject:cameraView.cameraImage];
        [pasteboard writeObjects:copiedObjects];
	}
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void) keyDown: (NSEvent *) theEvent 
{ 
    NSLog(@"Hello World"); 
} 
@end
