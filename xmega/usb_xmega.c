// Minimal USB Stack for ATxmega32a4u and related
// http://nonolithlabs.com
// (C) 2011 Kevin Mehall (Nonolith Labs) <km@kevinmehall.net>
//
// Heavily borrows from LUFA
// Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)
//
// Licensed under the terms of the GNU GPLv3+

#include <avr/io.h>

#define DEFINE_EVENT_ALIASES
#include "usb.h"

uint8_t ep0_buf_in[USB_EP0_SIZE];
uint8_t ep0_buf_out[USB_EP0_SIZE];
USB_EP_pair_t endpoints[USB_NUM_EP+1] GCC_FORCE_ALIGN_2;


volatile uint8_t USB_DeviceState;
volatile uint8_t USB_Device_ConfigurationNumber;

void USB_Init(){
	//uint_reg_t CurrentGlobalInt = GetGlobalInterruptMask();
	//GlobalInterruptDisable();

	NVM.CMD  = NVM_CMD_READ_CALIB_ROW_gc;
	USB.CAL0 = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, USBCAL0));
	NVM.CMD  = NVM_CMD_READ_CALIB_ROW_gc;
	USB.CAL1 = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, USBCAL1));

	//SetGlobalInterruptMask(CurrentGlobalInt);

	USB_ResetInterface();	
}

void USB_ResetInterface(){

	//if (USB_Options & USB_DEVICE_OPT_LOWSPEED)
	//  CLK.USBCTRL = ((((F_USB / 6000000) - 1) << CLK_USBPSDIV_gp) | CLK_USBSRC_RC32M_gc | CLK_USBSEN_bm);
	//else
	CLK.USBCTRL = ((((F_USB / 48000000) - 1) << CLK_USBPSDIV_gp) | CLK_USBSRC_RC32M_gc | CLK_USBSEN_bm);
	USB.EPPTR = (unsigned) &endpoints;
	USB.ADDR = 0;
	
	endpoints[0].out.STATUS = 0;
	endpoints[0].out.CTRL = USB_EP_TYPE_CONTROL_gc | USB_EP_size_to_gc(USB_EP0_SIZE);
	endpoints[0].out.DATAPTR = (unsigned) &ep0_buf_out;
	endpoints[0].in.STATUS = USB_EP_BUSNACK0_bm;
	endpoints[0].in.CTRL = USB_EP_TYPE_CONTROL_gc | USB_EP_size_to_gc(USB_EP0_SIZE);
	endpoints[0].in.DATAPTR = (unsigned) &ep0_buf_in;
	
	USB.CTRLA = USB_ENABLE_bm | USB_SPEED_bm | (USB_NUM_EP+1);
	
	USB_Attach();
}

const uint8_t* USB_ep0_from_progmem(const uint8_t* addr, uint16_t size) {
	uint8_t *buf = ep0_buf_in;
	uint16_t remaining = size;
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
	while (remaining--){
		*buf++ = pgm_read_byte(addr++);
	}
	return ep0_buf_in;
}

void USB_ConfigureClock(){
	// Configure DFLL for 48MHz, calibrated by USB SOF
	OSC.DFLLCTRL = OSC_RC32MCREF_USBSOF_gc;
	NVM.CMD  = NVM_CMD_READ_CALIB_ROW_gc;
	DFLLRC32M.CALB = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, USBRCOSC));
	DFLLRC32M.COMP1 = 0x1B; //Xmega AU manual, 4.17.19
	DFLLRC32M.COMP2 = 0xB7;
	DFLLRC32M.CTRL = DFLL_ENABLE_bm;
	
	CCP = CCP_IOREG_gc; //Security Signature to modify clock 
    OSC.CTRL = OSC_RC32MEN_bm | OSC_RC2MEN_bm; // enable internal 32MHz oscillator
    
    while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator ready
    
    OSC.PLLCTRL = OSC_PLLSRC_RC2M_gc | 16; // 2MHz * 16 = 32MHz
    
    CCP = CCP_IOREG_gc;
    OSC.CTRL = OSC_RC32MEN_bm | OSC_PLLEN_bm | OSC_RC2MEN_bm ; // Enable PLL
    
    while(!(OSC.STATUS & OSC_PLLRDY_bm)); // wait for PLL ready
    
    DFLLRC2M.CTRL = DFLL_ENABLE_bm;

    CCP = CCP_IOREG_gc; //Security Signature to modify clock 
    CLK.CTRL = CLK_SCLKSEL_PLL_gc; // Select PLL
    CLK.PSCTRL = 0x00; // No peripheral clock prescaler
}

ISR(USB_BUSEVENT_vect){
	if (USB.INTFLAGSACLR & USB_SOFIF_bm){
		USB.INTFLAGSACLR = USB_SOFIF_bm;
	}else if (USB.INTFLAGSACLR & (USB_CRCIF_bm | USB_UNFIF_bm | USB_OVFIF_bm)){
		USB.INTFLAGSACLR = (USB_CRCIF_bm | USB_UNFIF_bm | USB_OVFIF_bm);
	}else if (USB.INTFLAGSACLR & USB_STALLIF_bm){
		USB.INTFLAGSACLR = USB_STALLIF_bm;
	}else{
		USB.INTFLAGSACLR = USB_SUSPENDIF_bm | USB_RESUMEIF_bm | USB_RSTIF_bm;
		if (USB.STATUS & USB_BUSRST_bm){
			USB.STATUS &= ~USB_BUSRST_bm;
			USB_Init();
		}
	}
}

ISR(USB_TRNCOMPL_vect){
	USB.FIFOWP = 0;
	USB.INTFLAGSBCLR = USB_SETUPIF_bm | USB_TRNIF_bm;

	// Read once to prevent race condition where SETUP packet is interpreted as OUT
	uint8_t status = endpoints[0].out.STATUS;
	if (status & USB_EP_SETUP_bm){
		// TODO: race conditions because we can't block a setup packet
		LACR16(&(endpoints[0].out.STATUS), USB_EP_TRNCOMPL0_bm | USB_EP_SETUP_bm);
		memcpy(&usb_setup, ep0_buf_out, sizeof(usb_setup));
		usb_handle_setup();
	}else if(status & USB_EP_TRNCOMPL0_bm){
		USB_ep_clear_done(0);
		usb_handle_control_out_complete();
	}

	if (endpoints[0].in.STATUS & USB_EP_TRNCOMPL0_bm) {
		USB_ep_clear_done(0x80);
		usb_handle_control_in_complete();
	}
}
