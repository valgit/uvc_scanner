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
#import "UVCProbeWindowController.h"
#import "UVCCameraProber.h"
#import "UVCCamera.h"
#import "UVCCameraWindowController.h"
#import "UVCCapAppDelegate.h"

@interface UVCProductVendorPair : NSObject {
	uint16_t vendor;
	uint16_t product;
	NSString *vendorName;
	NSString *productName;
}
@property (assign) uint16_t vendor;
@property (assign) uint16_t product;
@property (copy) NSString *vendorName;
@property (copy) NSString *productName;
@end

@implementation UVCProductVendorPair
@synthesize vendor, product;
@synthesize vendorName, productName;
- (void) dealloc {
	[vendorName release];
	[productName release];
	[super dealloc];
}
@end

@implementation UVCProbeWindowController
- (id) init {
	self = [super init];
	if(self) {
		
	}
	return self;
}
- (void) windowDidLoad {
	[self refresh:self];
}
- (IBAction) refresh:(id)sender {
	UVCCameraProber *prober = [UVCCameraProber prober];
	NSUInteger count = [prober count];

	
	NSMutableArray *vp = [NSMutableArray arrayWithCapacity:count];
	for(unsigned i=0; i<count; ++i) {
		UVCProductVendorPair *pvp = [[UVCProductVendorPair alloc] init];
		pvp.vendor = [prober vendorForIndex:i];
		pvp.product = [prober productForIndex:i];
		pvp.vendorName = [prober vendorNameForIndex:i];
		pvp.productName = [prober productNameForIndex:i];
		[vp addObject:pvp];
		[pvp release];
	}
	[vpList release];
	vpList = [vp retain];
	[probeList reloadData];
}

- (IBAction) showCamera:(id)sender {
	int idx = [probeList selectedRow];
	if(-1==idx) {
		return;
	}
	UVCProductVendorPair *pvp = [vpList objectAtIndex:idx];
	try {
		UVCCamera *cam = [[UVCCamera alloc] initWithVendor:pvp.vendor product:pvp.product];
		
		
		UVCCameraWindowController *cwin = [[UVCCameraWindowController alloc] initWithWindowNibName:@"CameraWindow" Camera:cam];
		[cam release]; // i dont need it
		[cwin showWindow:sender];
	} catch(std::exception &e) {
		NSAlert *alrt = [NSAlert alertWithMessageText:@"Camera acquisition failed" 
										defaultButton:@"OK" 
									  alternateButton:nil 
										  otherButton:nil 
							informativeTextWithFormat:[NSString stringWithCString:e.what() encoding:NSASCIIStringEncoding]
						 ];
		[alrt runModal];
	}
}

#pragma mark -
#pragma mark TableView Datasource

- (id)tableView:(NSTableView *)aTableView
objectValueForTableColumn:(NSTableColumn *)aTableColumn
			row:(NSInteger)rowIndex
{
	UVCProductVendorPair *pvp = [vpList objectAtIndex:rowIndex];
	
	if([[aTableColumn identifier] isEqualToString:@"vend"]) {
		return [NSString stringWithFormat:@"0x%04X", pvp.vendor];
	}
	if([[aTableColumn identifier] isEqualToString:@"prod"]) {
		return [NSString stringWithFormat:@"0x%04X", pvp.product];
	}
	if([[aTableColumn identifier] isEqualToString:@"vendname"]) {
		return pvp.vendorName;
	}
	if([[aTableColumn identifier] isEqualToString:@"prodname"]) {
		return pvp.productName;
	}
	return @"undefined";
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
    return [vpList count];
}

@end
