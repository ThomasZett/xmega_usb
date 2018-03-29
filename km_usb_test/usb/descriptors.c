/* descriptors.c
 *
 * Copyright 2018 Paul Qureshi
 *
 * USB descriptors and handlers
 */

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "usb_xmega.h"
#include "dfu.h"
#include "usb_config.h"

// Notes:
// Fill in msft_extended for WCID

USB_ENDPOINTS(1);


/**************************************************************************************************
* USB Device descriptor
*/
const __flash USB_DeviceDescriptor_t device_descriptor = {
	.bLength				= sizeof(USB_DeviceDescriptor_t),
	.bDescriptorType		= USB_DTYPE_Device,

	.bcdUSB                 = 0x0200,
#ifdef USB_HID
	.bDeviceClass           = USB_CSCP_NoDeviceClass,
#else
	.bDeviceClass           = USB_CSCP_VendorSpecificClass,
#endif
	.bDeviceSubClass        = USB_CSCP_NoDeviceSubclass,
	.bDeviceProtocol        = USB_CSCP_NoDeviceProtocol,

	.bMaxPacketSize0        = USB_EP0_MAX_PACKET_SIZE,
	.idVendor               = USB_VID,
	.idProduct              = USB_PID,
	.bcdDevice              = (USB_VERSION_MAJOR << 8) | (USB_VERSION_MINOR),

	.iManufacturer          = 0x01,
	.iProduct               = 0x02,
#ifdef USB_SERIAL_NUMBER
	.iSerialNumber          = 0x03,
#else
	.iSerialNumber          = 0x00,
#endif

	.bNumConfigurations     = 1
};


/**************************************************************************************************
* HID report descriptor
*/
#ifdef USB_HID
const __flash uint8_t hid_report_descriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x00,                    // COLLECTION (Physical)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x08,                    //   USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //   USAGE (X)
    0x09, 0x31,                    //   USAGE (Y)
    0x15, 0x81,                    //   LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x02,                    //   REPORT_COUNT (2)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0xc0                           // END_COLLECTION
};
#endif


/**************************************************************************************************
* USB device descriptor
*/
typedef struct {
	USB_ConfigurationDescriptor_t	Config;
	USB_InterfaceDescriptor_t		Interface0;
#ifdef USB_HID
	USB_HIDDescriptor_t			HIDDescriptor;
	USB_EndpointDescriptor_t		HIDEndpoint;
#else
	USB_EndpointDescriptor_t		DataInEndpoint;
	USB_EndpointDescriptor_t		DataOutEndpoint;
#endif
#ifdef USB_DFU_RUNTIME
	USB_InterfaceDescriptor_t		DFU_intf_runtime;
	DFU_FunctionalDescriptor_t	DFU_desc_runtime;
#endif
} ConfigDesc_t;


/**************************************************************************************************
* USB configuration descriptor
*/
const __flash ConfigDesc_t configuration_descriptor = {
	.Config = {
		.bLength = sizeof(USB_ConfigurationDescriptor_t),
		.bDescriptorType = USB_DTYPE_Configuration,
		.wTotalLength  = sizeof(ConfigDesc_t),
#ifdef USB_DFU_RUNTIME
		.bNumInterfaces = 2,
#else
		.bNumInterfaces = 1,
#endif
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = USB_CONFIG_ATTR_BUSPOWERED,
		.bMaxPower = USB_CONFIG_POWER_MA(100)
	},
#ifdef USB_HID
	.Interface0 = {
		.bLength = sizeof(USB_InterfaceDescriptor_t),
		.bDescriptorType = USB_DTYPE_Interface,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CSCP_HIDClass,
		.bInterfaceSubClass = USB_CSCP_HIDNoSubclass,
		.bInterfaceProtocol = USB_CSCP_HIDNoProtocol,
		.iInterface = 0
	},
	.HIDDescriptor = {
		.bLength = sizeof(USB_HIDDescriptor_t),
		.bDescriptorType = USB_DTYPE_HID,
		.bcdHID = 0x0111,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bReportDescriptorType = USB_DTYPE_Report,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	},
	.HIDEndpoint = {
		.bLength = sizeof(USB_EndpointDescriptor_t),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = 0x81,
		.bmAttributes = (USB_EP_TYPE_INTERRUPT),
		.wMaxPacketSize = 64,
		.bInterval = USB_HID_POLL_RATE_MS
	},
#else
	.Interface0 = {
		.bLength = sizeof(USB_InterfaceDescriptor_t),
		.bDescriptorType = USB_DTYPE_Interface,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_CSCP_VendorSpecificClass,
		.bInterfaceSubClass = 0x00,
		.bInterfaceProtocol = 0x00,
		.iInterface = 0
	},
	.DataInEndpoint = {
		.bLength = sizeof(USB_EndpointDescriptor_t),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = 0x81,
		.bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.wMaxPacketSize = 64,
		.bInterval = 0x00
	},
	.DataOutEndpoint = {
		.bLength = sizeof(USB_EndpointDescriptor_t),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = 0x2,
		.bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.wMaxPacketSize = 64,
		.bInterval = 0x00
	},
#endif
#ifdef USB_DFU_RUNTIME
	.DFU_intf_runtime = {
		.bLength = sizeof(USB_InterfaceDescriptor_t),
		.bDescriptorType = USB_DTYPE_Interface,
		.bInterfaceNumber = 1,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = DFU_INTERFACE_CLASS,
		.bInterfaceSubClass = DFU_INTERFACE_SUBCLASS,
		.bInterfaceProtocol = DFU_INTERFACE_PROTOCOL_DFUMODE,
		.iInterface = 0x10
	},
	.DFU_desc_runtime = {
		.bLength = sizeof(DFU_FunctionalDescriptor_t),
		.bDescriptorType = DFU_DESCRIPTOR_TYPE,
		.bmAttributes = (DFU_ATTR_CANDOWNLOAD_bm | DFU_ATTR_WILLDETACH_bm),
		.wDetachTimeout = 0,
		.wTransferSize = APP_SECTION_PAGE_SIZE,
		.bcdDFUVersion = 0x0101
	},
#endif
};


/**************************************************************************************************
 *	USB strings
 */
#define	CONCAT(a, b)	a##b
#define	USTRING(s)		CONCAT(u, s)

const __flash USB_StringDescriptor_t language_string = {
	.bLength = USB_STRING_LEN(1),
	.bDescriptorType = USB_DTYPE_String,
	.bString = {USB_LANGUAGE_EN_US},
};

const __flash USB_StringDescriptor_t manufacturer_string = {
	.bLength = USB_STRING_LEN(USB_STRING_MANUFACTURER),
	.bDescriptorType = USB_DTYPE_String,
	.bString = USTRING(USB_STRING_MANUFACTURER)
};

const __flash USB_StringDescriptor_t product_string = {
	.bLength = USB_STRING_LEN(USB_STRING_PRODUCT),
	.bDescriptorType = USB_DTYPE_String,
	.bString = USTRING(USB_STRING_PRODUCT)
};

#ifdef USB_DFU_RUNTIME
const __flash USB_StringDescriptor_t dfu_runtime_string = {
	.bLength = USB_STRING_LEN("Runtime"),
	.bDescriptorType = USB_DTYPE_String,
	.bString = u"Runtime"
};
#endif // USB_DFU_RUNTIME


/**************************************************************************************************
 *	Optional serial number
 */
#ifdef USB_SERIAL_NUMBER
/*
USB_StringDescriptor serial_string = {
	.bLength = 22*2,
	.bDescriptorType = USB_DTYPE_String,
	.bString = u"0123456789ABCDEFGHIJKL"
};*/

void byte2char16(uint8_t byte, __CHAR16_TYPE__ *c)
{
	*c++ = (byte >> 4) < 10 ? (byte >> 4) + '0' : (byte >> 4) + 'A';
	*c = (byte & 0xF) < 10 ? (byte & 0xF) + '0' : (byte & 0xF) + 'A';

	// this version uses less flash memory
	//*c++ = 'A' + (byte >> 4);
	//*c = 'A' + (byte & 0xF);
}

uint8_t read_calibration_byte(uint16_t address)
{
	NVM.CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t res;
	__asm__ ("lpm %0, Z\n" : "=r" (res) : "z" (address));
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
	return res;
}

void generate_serial(void)
{
	USB_StringDescriptor_t *serial_string = (USB_StringDescriptor_t *)ep0_buf_in;
	serial_string->bDescriptorType = USB_DTYPE_String;
	serial_string->bLength = 22*2;

	__CHAR16_TYPE__ *c = (__CHAR16_TYPE__ *)&serial_string->bString;
	uint8_t idx = offsetof(NVM_PROD_SIGNATURES_t, LOTNUM0);
	for (uint8_t i = 0; i < 6; i++)
	{
		byte2char16(read_calibration_byte(idx++), c);
		c += 2;
	}
	byte2char16(read_calibration_byte(offsetof(NVM_PROD_SIGNATURES_t, WAFNUM)), c);
	c += 2;
	idx = offsetof(NVM_PROD_SIGNATURES_t, COORDX0);
	for (uint8_t i = 0; i < 4; i++)
	{
		byte2char16(read_calibration_byte(idx++), c);
		c += 2;
	}
}
#endif


/**************************************************************************************************
 *	Optional Microsoft WCID stuff
 */
#ifdef USB_WCID
const __flash USB_StringDescriptor_t msft_string = {
	.bLength = 18,
	.bDescriptorType = USB_DTYPE_String,
	.bString = u"MSFT100" WCID_REQUEST_ID_STR
};

__attribute__((__aligned__(2))) const USB_MicrosoftCompatibleDescriptor_t msft_compatible = {
	.dwLength = sizeof(USB_MicrosoftCompatibleDescriptor_t) +
				1*sizeof(USB_MicrosoftCompatibleDescriptor_Interface_t),
	.bcdVersion = 0x0100,
	.wIndex = 0x0004,
	.bCount = 1,
	.reserved = {0, 0, 0, 0, 0, 0, 0},
	.interfaces = {
		{
#ifdef USB_HID
			.bFirstInterfaceNumber = 1,		// WCID only needed for the DFU interface
#else
			.bFirstInterfaceNumber = 0,		// WCID covers both interfaces
#endif
			.reserved1 = 0x01,
			.compatibleID = "WINUSB\0\0",
			.subCompatibleID = {0, 0, 0, 0, 0, 0, 0, 0},
			.reserved2 = {0, 0, 0, 0, 0, 0},
		},
	}
};

#ifdef USB_WCID_EXTENDED
// example of two extended properties
__attribute__((__aligned__(2))) const USB_MicrosoftExtendedPropertiesDescriptor_t msft_extended = {
	.dwLength = sizeof(USB_MicrosoftExtendedPropertiesDescriptor_t),
	.bcdVersion = 0x0100,
	.wIndex = 0x0005,
	.wCount = 2,

	.dwPropLength = 132,
	.dwType = 1,
	.wNameLength = 40,
	.name = L"DeviceInterfaceGUID\0",
	.dwDataLength = 78,
	.data = L"{42314231-5A81-49F0-BC3D-A4FF138216D7}\0",

	.dwPropLength2 = 14 + (6*2) + (13*2),
	.dwType2 = 1,
	.wNameLength2 = 6*2,
	.name2 = L"Label\0",
	.dwDataLength2 = 13*2,
	.data2 = L"Name56789AB\0",
};

/*
// example of one extended property (WinUSB GUID)
__attribute__((__aligned__(4))) const USB_MicrosoftExtendedPropertiesDescriptor msft_extended = {
	.dwLength = sizeof(USB_MicrosoftExtendedPropertiesDescriptor),
	.dwLength = 142,
	.bcdVersion = 0x0100,
	.wIndex = 0x0005,
	.wCount = 1,
	.dwPropLength = 132,
	.dwType = 1,
	.wNameLength = 40,
	.name = L"DeviceInterfaceGUID\0",
	.dwDataLength = 78,
	.data = L"{42314231-5A81-49F0-BC3D-A4FF138216D7}\0",
};
*/
/*
// example of one extended property (WinUSB GUID) using "DeviceInterfaceGUIDs" (plural)
// see "important note 2" at https://github.com/pbatard/libwdi/wiki/WCID-Devices#Defining_a_Device_Interface_GUID_or_other_device_specific_properties
__attribute__((__aligned__(4))) const USB_MicrosoftExtendedPropertiesDescriptor msft_extended = {
	.dwLength = sizeof(USB_MicrosoftExtendedPropertiesDescriptor),
	.dwLength = 146,
	.bcdVersion = 0x0100,
	.wIndex = 0x0005,
	.wCount = 1,
	.dwPropLength = 136,
	.dwType = 7,
	.wNameLength = 42,
	.name = L"DeviceInterfaceGUIDs\0",
	.dwDataLength = 80,
	.data = L"{42314231-5A81-49F0-BC3D-A4FF138216D7}\0\0",
};
*/
#endif // USB_WCID_EXTENDED

void handle_msft_compatible(void)
{
	const uint8_t *data;
	uint16_t len;
#ifdef USB_WCID_EXTENDED
	if (usb_setup.wIndex == 0x0005) {
		len = msft_extended.dwLength;
		data = (const uint8_t *)&msft_extended;
	} else
#endif
	if (usb_setup.wIndex == 0x0004) {
		len = msft_compatible.dwLength;
		data = (const uint8_t *)&msft_compatible;
	} else {
		return usb_ep0_stall();
	}
	if (len > usb_setup.wLength) {
		len = usb_setup.wLength;
	}
	usb_ep_start_in(0x80, data, len, true);
	usb_ep0_out();
}
#endif // USB_WCID


/**************************************************************************************************
 *	USB descriptor request handler
 */
uint16_t usb_handle_descriptor_request(uint8_t type, uint8_t index) {
	const void* address = NULL;
	uint16_t size = 0;

	uint8_t cmd_backup = NVM.CMD;
	NVM.CMD = 0;

	switch (type)
	{
		case USB_DTYPE_Device:
			address = &device_descriptor;
			size    = sizeof(USB_DeviceDescriptor_t);
			break;
		case USB_DTYPE_Configuration:
			address = &configuration_descriptor;
			size    = sizeof(ConfigDesc_t);
			break;
#ifdef USB_HID
		case USB_DTYPE_HID:
			address = &configuration_descriptor.HIDDescriptor;
			size	= sizeof(USB_HIDDescriptor_t);
			break;
		case USB_DTYPE_Report:
			address = &hid_report_descriptor;
			size    = sizeof(hid_report_descriptor);
			break;
#endif
		case USB_DTYPE_String:
			switch (index)
			{
				case 0x00:
					address = &language_string;
					break;
				case 0x01:
					address = &manufacturer_string;
					break;
				case 0x02:
					address = &product_string;
					break;
#ifdef USB_SERIAL_NUMBER
				case 0x03:
					generate_serial();
					return sizeof(USB_StringDescriptor_t) + (22*2);
#endif
#ifdef USB_DFU_RUNTIME
				case 0x10:
					address = &dfu_runtime_string;
					break;
#endif
#ifdef USB_WCID
				case 0xEE:
					address = &msft_string;
					break;
#endif

				default:
					return 0;
			}
			size = pgm_read_byte(&((USB_StringDescriptor_t*)address)->bLength);
			break;
	}

	for (uint8_t i = 0; i < size; i++)
		ep0_buf_in[i] = pgm_read_byte(address++);
	NVM.CMD = cmd_backup;
	return size;
}


/**************************************************************************************************
 *	Set USB configuration
 */
bool usb_cb_set_configuration(uint8_t config) {
	if (config <= 1) {
		return true;
	} else {
		return false;
	}
}

/**************************************************************************************************
 *	Set USB interface
 */
bool usb_cb_set_interface(uint16_t interface, uint16_t altsetting) {
	return false;
}