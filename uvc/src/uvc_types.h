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
#ifndef UVC_TYPES_H__
#define UVC_TYPES_H__
namespace uvc {


#pragma pack(push, 1)
	struct probe {
		uint16_t bmHint;
		uint8_t bFormatIndex;
		uint8_t bFrameIndex;
		uint32_t dwFrameInterval;
		uint16_t wKeyFrameRate;
		uint16_t wPFrameRate;
		uint16_t wCompQuality;
		uint16_t wCompWindowSize;
		uint16_t wDelay;
		uint32_t dwMaxVideoFrameSize;
		uint32_t dwMaxPayloadTransferSize;
		uint32_t dwClockFrequency;
		uint8_t bmFramingInfo;
		uint8_t bPreferedVersion;
		uint8_t bMinVersion;
		uint8_t bMaxVersion;
	};
#pragma pack(pop)


enum 
{
	INPUT_TERMINAL_ID = 0x01,

	USB_INTERFACE = 0x04,
	USB_ENDPOINT = 0x05,

	INTERFACE_CLASS_VIDEO	= 0x0e, //14
	INTERFACE_SUBCLASS_CONTROL = 0x01,
	INTERFACE_SUBCLASS_STREAMING = 0x02,

    // Video Interface Subclass Codes
    //
    SC_UNDEFINED		= 0x00,
    SC_VIDEOCONTROL		= 0x01,
    SC_VIDEOSTREAMING		= 0x02,

    // Video Interface Protocol Codes
    //
    PC_PROTOCOL_UNDEFINED	= 0x00,

    // Video Class Specific Descriptor Types
    //
    CS_UNDEFINED		= 0x20,
    CS_DEVICE			= 0x21,
    CS_CONFIGURATION	= 0x22,
    CS_STRING			= 0x23,
    CS_INTERFACE		= 0x24,
    CS_ENDPOINT			= 0x25,

    // Video Class Specific Control Interface Descriptor Types
    //
    VC_DESCRIPTOR_UNDEFINED	= 0x00,
    VC_HEADER			= 0x01,
    VC_INPUT_TERMINAL		= 0x02,
    VC_OUTPUT_TERMINAL		= 0x03,
    VC_SELECTOR_UNIT		= 0x04,
    VC_PROCESSING_UNIT		= 0x05,
    VC_EXTENSION_UNIT		= 0x06,

    // Video Class Specific Streaming Interface Descriptor Types
    //
    VS_UNDEFINED		= 0x00,
    VS_INPUT_HEADER		= 0x01,
    VS_OUTPUT_HEADER		= 0x02,
    VS_STILL_IMAGE_FRAME	= 0x03,
    VS_FORMAT_UNCOMPRESSED	= 0x04,
    VS_FRAME_UNCOMPRESSED	= 0x05,
    VS_FORMAT_MJPEG		= 0x06,
    VS_FRAME_MJPEG		= 0x07,
    VS_FORMAT_MPEG1		= 0x08,
    VS_FORMAT_MPEG2PS		= 0x09,
    VS_FORMAT_MPEG2TS		= 0x0a,
    VS_FORMAT_MPEG4SL		= 0x0b,
    VS_FORMAT_DV		= 0x0c,
    VS_FORMAT_VENDOR		= 0x0d,
    VS_FRAME_VENDOR		= 0x0e,
	VS_COLORFORMAT	= 0x0d,

    // Video Class Specific Endpoint Descriptor Subtypes
    //
    EP_UNDEFINED		= 0x00,
    EP_GENERAL			= 0x01,
    EP_ENDPOINT			= 0x02,
    EP_INTERRUPT		= 0x03,

    // Video Class Specific Request Codes
    //
    RC_UNDEFINED		= 0x00,
    SET_CUR			= 0x01,
    GET_CUR			= 0x81,
    GET_MIN			= 0x82,
    GET_MAX			= 0x83,
    GET_RES			= 0x84,
    GET_LEN			= 0x85,
    GET_INFO			= 0x86,
    GET_DEF			= 0x87,

    // Video Control Interface Control Selectors
    //
    VC_UNDEFINED_CONTROL	= 0x00,
    VC_VIDEO_POWER_MODE_CONTROL	= 0x01,
    VC_REQUEST_ERROR_CODE_CONTROL		= 0x02,
    VC_REQUEST_INDICATE_HOST_CLOCK_CONTROL	= 0x03,

    // Terminal Control Selectors
    //
    TE_CONTROL_UNDEFINED	= 0x00,

    // Selector Unit Control Selectors
    //
    SU_CONTROL_UNDEFINED	= 0x00,
    SU_INPUT_SELECT_CONTROL	= 0x01,

    // Camera Terminal Control Selectors
    //
    CT_CONTROL_UNDEFINED		= 0x00,
    CT_SCANNING_MODE_CONTROL		= 0x01,
    CT_AE_MODE_CONTROL			= 0x02,
    CT_AE_PRIORITY_CONTROL		= 0x03,
    CT_EXPOSURE_TIME_ABSOLUTE_CONTROL	= 0x04,
    CT_EXPOSURE_TIME_RELATIVE_CONTROL	= 0x05,
    CT_FOCUS_ABSOLUTE_CONTROL		= 0x06,
    CT_FOCUS_RELATIVE_CONTROL		= 0x07,
    CT_FOCUS_AUTO_CONTROL		= 0x08,
    CT_IRIS_ABSOLUTE_CONTROL		= 0x09,
    CT_IRIS_RELATIVE_CONTROL		= 0x0A,
    CT_ZOOM_ABSOLUTE_CONTROL 		= 0x0B,
    CT_ZOOM_RELATIVE_CONTROL		= 0x0C,
    CT_PANTILT_ABSOLUTE_CONTROL		= 0x0D,
    CT_PANTILT_RELATIVE_CONTROL		= 0x0E,
    CT_ROLL_ABSOLUTE_CONTROL		= 0x0F,
    CT_ROLL_RELATIVE_CONTROL		= 0x10,

    // Processing Unit Control Selectors
    //
    PU_CONTROL_UNDEFINED		= 0x00,
    PU_BACKLIGHT_COMPENSATION_CONTROL	= 0x01,
    PU_BRIGHTNESS_CONTROL		= 0x02,
    PU_CONTRAST_CONTROL			= 0x03,
    PU_GAIN_CONTROL			= 0x04,
    PU_POWER_LINE_FREQUENCY_CONTROL	= 0x05,
    PU_HUE_CONTROL			= 0x06,
    PU_SATURATION_CONTROL		= 0x07,
    PU_SHARPNESS_CONTROL		= 0x08,
    PU_GAMMA_CONTROL			= 0x09,
    PU_WHITE_BALANCE_TEMPERATURE_CONTROL	= 0x0A,
    PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL	= 0x0B,
    PU_WHITE_BALANCE_COMPONENT_CONTROL		= 0x0C,
    PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL	= 0x0D,
    PU_DIGITAL_MULTIPLIER_CONTROL		= 0x0E,
    PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL		= 0x0F,
    PU_HUE_AUTO_CONTROL				= 0x10,

    // Extension Unit Control Selectors
    //
    XU_CONTROL_UNDEFINED		= 0x00,
    XU_ENABLE_CONTROL			= 0x01,

    // Video Streaming Interface Control Selectors
    //
    VS_CONTROL_UNDEFINED		= 0x00,
    VS_PROBE_CONTROL			= 0x01,
    VS_COMMIT_CONTROL			= 0x02,
    VS_STILL_PROBE_CONTROL		= 0x03,
    VS_STILL_COMMIT_CONTROL		= 0x04,
    VS_STILL_IMAGE_TRIGGER_CONTROL	= 0x05,
    VS_STREAM_ERROR_CODE_CONTROL	= 0x06,
    VS_GENERATE_KEY_FRAME_CONTROL	= 0x07,
    VS_UPDATE_FRAME_SEGMENT_CONTROL	= 0x08,
    VS_SYNCH_DELAY_CONTROL		= 0x09,

    // USB Terminal Types
    //
    TT_VENDOR_SPECIFIC			= 0x0100,
    TT_STREAMING			= 0x0101,

    // Input Terminal Types
    //
    ITT_VENDOR_SPECIFIC			= 0x0200,
    ITT_CAMERA				= 0x0201,
    ITT_MEDIA_TRANSPORT_UNIT		= 0x0202,

    // Output Terminal Types
    //
    OTT_VENDOR_SPECIFIC			= 0x0300,
    OTT_DISPLAY				= 0x0301,
    OTT_MEDIA_TRANSPORT_OUTPUT		= 0x0302,

    // External Terminal Types
    //
    EXTERNAL_VENDOR_SPECIFIC		= 0x0400,
    COMPOSITE_CONNECTOR			= 0x0401,
    SVIDEO_CONNECTOR			= 0x0402,
    COMPONENT_CONNECTOR			= 0x0403,

	//VS_FORMAT_UNCOMPRESSED = 0x04,
	

};
}
#endif /* UVC_TYPES_H__ */

