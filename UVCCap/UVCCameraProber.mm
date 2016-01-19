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
#import "UVCCameraProber.h"


@implementation UVCCameraProber
+ (id) prober {
	return [[[UVCCameraProber alloc] init] autorelease];
}
- (id) init {
	self = [super init];
	if(self) {
		/** 0xe is UVC **/
		enumerator = new usb::devices_of_class_enumerator(0xe);
	}
	return self;
}
- (void) dealloc {
	NSLog(@"UVCCameraProber dealloc");
	if(enumerator) {
		delete enumerator;
	}
	[super dealloc];
}
- (NSUInteger) count {
	return enumerator->count();
}
- (NSUInteger) vendorForIndex:(NSUInteger)idx {
	uint16_t vendor;
	uint16_t product;
	enumerator->get(idx, vendor, product);
	return vendor;
}
- (NSUInteger) productForIndex:(NSUInteger)idx {
	uint16_t vendor;
	uint16_t product;
	enumerator->get(idx, vendor, product);
	return product;
}

- (NSString*) vendorNameForIndex:(NSUInteger)idx {
	const char *ptr = (const char*)enumerator->get_manufacturer_utf16(idx);
	if(!ptr) return @"";
	int len = ptr[0];

	NSString *str = [[[NSString alloc] initWithBytes:ptr+1 length:len 
											encoding:NSUTF16StringEncoding] autorelease];
	return str;
}
- (NSString*) productNameForIndex:(NSUInteger)idx {
	const char *ptr = (const char*)enumerator->get_product_utf16(idx);
	if(!ptr) return @"";
	int len = ptr[0];
	
	NSString *str = [[[NSString alloc] initWithBytes:ptr+1 length:len 
											encoding:NSUTF16StringEncoding] autorelease];
	return str;
}
@end
