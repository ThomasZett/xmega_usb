#include <avr/pgmspace.h>
#include <string.h>
#include "xmega/usb_xmega.h"
#include "dfu.h"
#include "usb_config.h"

// Notes:
// Fill in VID/PID in device_descriptor
// Fill in msft_extended for WCID
// WCID request ID can be changed below
// Other options in usb.h
// Additional compiler flags: -std=gnu99 -fno-strict-aliasing -Wstrict-prototypes -fno-jump-tables

#define WCID_REQUEST_ID			0x22
#define WCID_REQUEST_ID_STR		u"\x22"

USB_ENDPOINTS(1);

/**************************************************************************************************
 *	USB Device descriptor
 */
const USB_DeviceDescriptor PROGMEM device_descriptor = {
	.bLength				= sizeof(USB_DeviceDescriptor),
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
 *	HID descriptor
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
 *	USB Configuration descriptor
 */
/*
typedef struct ConfigDesc {
	USB_ConfigurationDescriptor Config;
	USB_InterfaceDescriptor Interface0;
	USB_EndpointDescriptor DataInEndpoint;
	USB_EndpointDescriptor DataOutEndpoint;
#ifdef USB_DFU_RUNTIME
	USB_InterfaceDescriptor DFU_intf_runtime;
	DFU_FunctionalDescriptor DFU_desc_runtime;
#endif
} ConfigDesc;

const __flash ConfigDesc configuration_descriptor = {
	.Config = {
		.bLength = sizeof(USB_ConfigurationDescriptor),
		.bDescriptorType = USB_DTYPE_Configuration,
		.wTotalLength  = sizeof(ConfigDesc),
#ifndef USB_DFU_RUNTIME
		.bNumInterfaces = 1,
#else
		.bNumInterfaces = 2,
#endif
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = USB_CONFIG_ATTR_BUSPOWERED,
		.bMaxPower = USB_CONFIG_POWER_MA(500)
	},
	.Interface0 = {
		.bLength = sizeof(USB_InterfaceDescriptor),
		.bDescriptorType = USB_DTYPE_Interface,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,			// !!! CHECK !!!
		.bInterfaceClass = USB_CSCP_VendorSpecificClass,
		.bInterfaceSubClass = 0x00,
		.bInterfaceProtocol = 0x00,
		.iInterface = 0
	},
	.DataInEndpoint = {
		.bLength = sizeof(USB_EndpointDescriptor),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = 0x81,
		.bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.wMaxPacketSize = 64,
		.bInterval = 0x00
	},
	.DataOutEndpoint = {
		.bLength = sizeof(USB_EndpointDescriptor),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = 0x2,
		.bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.wMaxPacketSize = 64,
		.bInterval = 0x00
	},
#ifdef USB_DFU_RUNTIME
	.DFU_intf_runtime = {
		.bLength = sizeof(USB_InterfaceDescriptor),
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
		.bLength = sizeof(DFU_FunctionalDescriptor),
		.bDescriptorType = DFU_DESCRIPTOR_TYPE,
		.bmAttributes = (DFU_ATTR_CANDOWNLOAD_bm | DFU_ATTR_WILLDETACH_bm),
		.wDetachTimeout = 0,
		.wTransferSize = APP_SECTION_PAGE_SIZE,
		.bcdDFUVersion = 0x0101
	},
#endif
};
*/

typedef struct ConfigDesc {
	USB_ConfigurationDescriptor Config;
	USB_InterfaceDescriptor Interface0;
	//USB_EndpointDescriptor DataInEndpoint;
	//USB_EndpointDescriptor DataOutEndpoint;
	USB_HIDDescriptor		HIDDescriptor;
	USB_EndpointDescriptor	HIDEndpoint;
#ifdef USB_DFU_RUNTIME
	USB_InterfaceDescriptor DFU_intf_runtime;
	DFU_FunctionalDescriptor DFU_desc_runtime;
#endif
} ConfigDesc;

const __flash ConfigDesc configuration_descriptor = {
	.Config = {
		.bLength = sizeof(USB_ConfigurationDescriptor),
		.bDescriptorType = USB_DTYPE_Configuration,
		.wTotalLength  = sizeof(ConfigDesc),
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
	.Interface0 = {
		.bLength = sizeof(USB_InterfaceDescriptor),
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
		.bLength = sizeof(USB_HIDDescriptor),
		.bDescriptorType = USB_DTYPE_HID,
		.bcdHID = 0x0111,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bReportDescriptorType = USB_DTYPE_Report,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	},
	.HIDEndpoint = {
		.bLength = sizeof(USB_EndpointDescriptor),
		.bDescriptorType = USB_DTYPE_Endpoint,
		.bEndpointAddress = 0x81,
		.bmAttributes = (USB_EP_TYPE_INTERRUPT),
		.wMaxPacketSize = 64,
		.bInterval = 0x08
	},
#ifdef USB_DFU_RUNTIME
	.DFU_intf_runtime = {
		.bLength = sizeof(USB_InterfaceDescriptor),
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
		.bLength = sizeof(DFU_FunctionalDescriptor),
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

const __flash USB_StringDescriptor language_string = {
	.bLength = USB_STRING_LEN(1),
	.bDescriptorType = USB_DTYPE_String,
	.bString = {USB_LANGUAGE_EN_US},
};

const __flash USB_StringDescriptor manufacturer_string = {
	.bLength = USB_STRING_LEN(USB_STRING_MANUFACTURER),
	.bDescriptorType = USB_DTYPE_String,
	.bString = USTRING(USB_STRING_MANUFACTURER)
};

const __flash USB_StringDescriptor product_string = {
	.bLength = USB_STRING_LEN(USB_STRING_PRODUCT),
	.bDescriptorType = USB_DTYPE_String,
	.bString = USTRING(USB_STRING_PRODUCT)
};


/**************************************************************************************************
 *	Optional serial number
 */
#ifdef USB_SERIAL_NUMBER
USB_StringDescriptor serial_string = {
	.bLength = 22*2,
	.bDescriptorType = USB_DTYPE_String,
	.bString = u"0000000000000000000000"
};

const __flash char hexlut[] = "0123456789ABCDEF";
void byte2char16(uint8_t byte, __CHAR16_TYPE__ *c)
{
	*c++ = hexlut[byte >> 4];
	*c = hexlut[byte & 0xF];

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
	static bool generated = false;
	if (generated) return;
	generated = true;

	__CHAR16_TYPE__ *c = (__CHAR16_TYPE__ *)&serial_string.bString;
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
 *	Optional DFU runtime interface
 */
#ifdef USB_DFU_RUNTIME
const __flash USB_StringDescriptor dfu_runtime_string = {
	.bLength = USB_STRING_LEN("Runtime"),
	.bDescriptorType = USB_DTYPE_String,
	.bString = u"Runtime"
};

void dfu_control_setup(void)
{
	switch (usb_setup.bRequest)
	{
		case DFU_DETACH:
			dfu_runtime_cb_enter_dfu_mode();
			return usb_ep0_out();

		// read status
		case DFU_GETSTATUS: {
			uint8_t len = usb_setup.wLength;
			if (len > sizeof(DFU_StatusResponse))
				len = sizeof(DFU_StatusResponse);
			DFU_StatusResponse *st = (DFU_StatusResponse *)ep0_buf_in;
			st->bStatus = DFU_STATUS_OK;
			st->bState = DFU_STATE_dfuIDLE;
			st->bwPollTimeout[0] = 0;
			st->bwPollTimeout[1] = 0;
			st->bwPollTimeout[2] = 0;
			st->iString = 0;
			usb_ep0_in(len);
			return usb_ep0_out();
		}

		// abort, clear status
		case DFU_ABORT:
		case DFU_CLRSTATUS:
			usb_ep0_in(0);
			return usb_ep0_out();

		// read state
		case DFU_GETSTATE:
			ep0_buf_in[0] = 0;
			usb_ep0_in(1);
			return usb_ep0_out();

		// unsupported requests
		default:
			return usb_ep0_stall();
	}
}
#endif // USB_DFU_RUNTIME


/**************************************************************************************************
 *	Optional Microsoft WCID stuff
 */
#ifdef USB_WCID
const __flash USB_StringDescriptor msft_string = {
	.bLength = 18,
	.bDescriptorType = USB_DTYPE_String,
	.bString = u"MSFT100" WCID_REQUEST_ID_STR
};

__attribute__((__aligned__(4))) const USB_MicrosoftCompatibleDescriptor msft_compatible = {
	.dwLength = sizeof(USB_MicrosoftCompatibleDescriptor) +
				1*sizeof(USB_MicrosoftCompatibleDescriptor_Interface),
	.bcdVersion = 0x0100,
	.wIndex = 0x0004,
	.bCount = 1,
	.reserved = {0, 0, 0, 0, 0, 0, 0},
	.interfaces = {
		{
			.bFirstInterfaceNumber = 0,
			.reserved1 = 0x01,
			.compatibleID = "WINUSB\0\0",
			.subCompatibleID = {0, 0, 0, 0, 0, 0, 0, 0},
			.reserved2 = {0, 0, 0, 0, 0, 0},
		},
	}
};

#ifdef USB_WCID_EXTENDED
__attribute__((__aligned__(4))) const USB_MicrosoftExtendedPropertiesDescriptor msft_extended = {
	.dwLength = sizeof(USB_MicrosoftExtendedPropertiesDescriptor),
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

void handle_msft_compatible(void) {
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
 *	USB request handling
 */
uint16_t usb_cb_get_descriptor(uint8_t type, uint8_t index) {
	const void* address = NULL;
	uint16_t size = 0;

	switch (type)
	{
		case USB_DTYPE_Device:
			address = &device_descriptor;
			size    = sizeof(USB_DeviceDescriptor);
			break;
		case USB_DTYPE_Configuration:
			address = &configuration_descriptor;
			size    = sizeof(ConfigDesc);
			break;
#ifdef USB_HID
		case USB_DTYPE_HID:
			address = &configuration_descriptor.HIDDescriptor;
			size	= sizeof(USB_HIDDescriptor);
			break;
		case USB_DTYPE_Report:
			address = &hid_report_descriptor;
			size    = sizeof(hid_report_descriptor);
			break;
#endif
		case USB_DTYPE_String:
			switch (index) {
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
					memcpy(ep0_buf_in, &serial_string, sizeof(serial_string));
					return serial_string.bLength;
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
			}
			size = pgm_read_byte(&((USB_StringDescriptor*)address)->bLength);
			break;
	}

	uint8_t cmd_backup = NVM.CMD;
	NVM.CMD = 0;
	for (uint8_t i = 0; i < size; i++)
		ep0_buf_in[i] = pgm_read_byte(address++);
	NVM.CMD = cmd_backup;
	return size;
}

bool usb_cb_set_configuration(uint8_t config) {
	if (config <= 1) {
		return true;
	} else {
		return false;
	}
}

void usb_cb_control_setup(void) {
	uint8_t recipient = usb_setup.bmRequestType & USB_REQTYPE_RECIPIENT_MASK;
	if (recipient == USB_RECIPIENT_DEVICE)
	{
		switch(usb_setup.bRequest)
		{
#ifdef USB_WCID
			case WCID_REQUEST_ID:
				return handle_msft_compatible();
#endif
		}
	}
	else if (recipient == USB_RECIPIENT_INTERFACE)
	{
		if (usb_setup.wIndex == 0)
		{			// main interface
			switch(usb_setup.bRequest)
			{
#ifdef USB_WCID_EXTENDED
				case WCID_REQUEST_ID:
					return handle_msft_compatible();
#endif
			}
		}
#ifdef USB_DFU_RUNTIME
		else if (usb_setup.wIndex == 1)		// DFU interface
			return dfu_control_setup();
#endif
	}

	return usb_ep0_stall();
}

bool usb_cb_set_interface(uint16_t interface, uint16_t altsetting) {
	return false;
}
