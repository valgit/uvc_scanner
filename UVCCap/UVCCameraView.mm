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
#import "UVCCameraView.h"
#import "UVCCamera.h"

@implementation UVCCameraView
//@synthesize camera;
@synthesize cameraImage;
@synthesize imgNum;
@synthesize compressionFactor;
@synthesize outputDirectory;
@synthesize format;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		
        
        // get the default output directory
        NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
        if (standardUserDefaults) {
            
            
            outputDirectory = [standardUserDefaults stringForKey:@"outputDirectory"];
            /*
                outputDirectory = NSHomeDirectory();
                outputDirectory = [outputDirectory stringByAppendingPathComponent:@"Pictures"];
            */
            
            imgNum = [[standardUserDefaults stringForKey:@"imgNum"] intValue];
            
            compressionFactor = [[standardUserDefaults stringForKey:@"outputQuality"] floatValue] / 100.0; 
            // should be an int value ... would be better
            format = NSTIFFFileType; // default type
            
            NSString *fileformat = [standardUserDefaults objectForKey:@"format"];
            if([fileformat isEqualToString:@"JPEG"]) {
                format = NSTIFFFileType;
                // ext = @"jpg";
            }
            if([fileformat isEqualToString:@"TIFF"]) {
                format = NSJPEGFileType;
                // ext = @"tiff";
            }
            if([fileformat isEqualToString:@"PNG"]) {
                format = NSPNGFileType;
                // ext = @"png";
            }
        }
        
    }
    return self;
}

- (void) dealloc {
	NSLog(@"Dealloc UVCCameraView");
	
	[super dealloc];
}
#if 0
- (void) beginStreaming {
	// set up camera to default (first) config
	if(NO==[camera setConfig:0]) {
		NSLog(@"Could not configure camera!");
		return;
	}
	NSLog(@"Camera configured for %dx%d",[camera width],[camera height]);
	// start capturing
	[camera beginCapture];
	// set-up timer for refreshing
	refreshTimer = [NSTimer scheduledTimerWithTimeInterval:0.001 
													target:self 
												  selector:@selector(fetchFrame:) 
												  userInfo:nil 
												   repeats:YES];
	[refreshTimer retain];
}

- (void) switchConfig:(unsigned)cfg {
	[refreshTimer invalidate];
	[refreshTimer release];	
	[camera endCapture];
	
	if(NO==[camera setConfig:cfg]) {
		NSLog(@"Could not configure camera!");
		return;
	}
	//NSLog(@"Camera configured for %dx%d",[camera width],[camera height]);
	// start capturing
	[camera beginCapture];
	// set-up timer for refreshing
	refreshTimer = [NSTimer scheduledTimerWithTimeInterval:0.001 
													target:self 
												  selector:@selector(fetchFrame:) 
												  userInfo:nil 
												   repeats:YES];
	[refreshTimer retain];
}

- (void) endStreaming {
	[refreshTimer invalidate];
	[refreshTimer release];	
	[camera endCapture];
	[camera release];
	camera = nil;
}
#endif
- (void)drawRect:(NSRect)dirtyRect {
    [[NSColor redColor] set];
    NSRectFill([self bounds]);
	
	if(cameraImage) {
		NSRect imageRect = NSMakeRect(0,0,0,0);
		imageRect.size = [cameraImage size];
		[cameraImage drawInRect:[self bounds] fromRect:imageRect operation:NSCompositeSourceOver fraction:1.0];
	}
	
}
#if 0
-(void)fetchFrame:(NSTimer*)timer {
	NSImage *img = [camera grabFrame];
	if(nil==img) {
		/* frame not updated yet, thus
		 simply return */
		return;
	}
	[cameraImage release];
	cameraImage = [img retain];
	
	[self setNeedsDisplay:YES];
}
#endif

# pragma mark First Responder Stuff
- (BOOL)acceptsFirstResponder
{
    return YES;
}

# pragma mark Keyboard Stuff
- (void)keyDown:(NSEvent *)event
{
    //unichar key = [[event charactersIgnoringModifiers] characterAtIndex:0];
    NSString *keyChar = [event characters];
	NSLog(@"key is  %d %@",[event keyCode],keyChar);

    if ( [event keyCode] == 49 ) {
         // Cache the reduced image
         NSData *imageData = [cameraImage TIFFRepresentation];
         NSBitmapImageRep *imageRep = [NSBitmapImageRep imageRepWithData:imageData];
         NSDictionary *imageProps = [NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:compressionFactor] forKey:NSImageCompressionFactor];
         imageData = [imageRep representationUsingType:NSTIFFFileType properties:imageProps];
        imgNum++;
        NSString *filename = [NSString stringWithFormat:@"%@/grab%d.tiff",outputDirectory,imgNum];
         [imageData writeToFile:filename atomically:NO];  
        //[filename release];
        NSLog(@"file write %@",filename);
        
	} else
	    [super keyDown:event];
}

@end
