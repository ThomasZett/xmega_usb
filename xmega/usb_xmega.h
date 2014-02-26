// Minimal USB Stack for ATxmega32a4u and related
// http://nonolithlabs.com
// (C) 2011 Kevin Mehall (Nonolith Labs) <km@kevinmehall.net>
//
// Heavily borrows from LUFA
// Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)
//
// Licensed under the terms of the GNU GPLv3+

#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define CPU_TO_LE16(x) x

/** Like __attribute__(align(2)), but actually works. 
    From http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=121033
 */
#define GCC_FORCE_ALIGN_2  __attribute__((section (".data,\"aw\",@progbits\n.p2align 1;")))
#define ATTR_ALWAYS_INLINE __attribute__ ((always_inline))

/// From Atmel: Macros for XMEGA instructions not yet supported by the toolchain
// Load and Clear 
#ifdef __GNUC__
#define LACR16(addr,msk) \
	__asm__ __volatile__ ( \
	"ldi r16, %1" "\n\t" \
	".dc.w 0x9306" "\n\t"\
	::"z" (addr), "M" (msk):"r16")
#else
	#define LACR16(addr,msk) __lac((unsigned char)msk,(unsigned char*)addr)
#endif
 
// Load and Set
#ifdef __GNUC__
#define LASR16(addr,msk) \
	__asm__ __volatile__ ( \
	"ldi r16, %1" "\n\t" \
	".dc.w 0x9305" "\n\t"\
	::"z" (addr), "M" (msk):"r16")
#else
#define LASR16(addr,msk) __las((unsigned char)msk,(unsigned char*)addr)
#endif

// Exchange
#ifdef __GNUC__
#define XCHR16(addr,msk) \
	__asm__ __volatile__ ( \
	"ldi r16, %1" "\n\t" \
	".dc.w 0x9304" "\n\t"\
	::"z" (addr), "M" (msk):"r16")
#else
#define XCHR16(addr,msk) __xch(msk,addr)
#endif

// Load and toggle
#ifdef __GNUC__
#define LATR16(addr,msk) \
	__asm__ __volatile__ ( \
	"ldi r16, %1" "\n\t" \
	".dc.w 0x9307" "\n\t"\
	::"z" (addr), "M" (msk):"r16")
#else
#define LATR16(addr,msk) __lat(msk,addr)
#endif

#ifndef USB_NUM_EP
	#define USB_NUM_EP 0
#endif

#ifndef USB_EP0_SIZE
	#define USB_EP0_SIZE 64
#endif

typedef union USB_EP_pair{
	union{
		struct{
			USB_EP_t out;
			USB_EP_t in;
		};
		USB_EP_t ep[2];
	};
} __attribute__((packed)) USB_EP_pair_t;

extern uint8_t ep0_buf_in[USB_EP0_SIZE];
extern uint8_t ep0_buf_out[USB_EP0_SIZE];
extern USB_EP_pair_t endpoints[USB_NUM_EP+1];

/** String descriptor index for the device's unique serial number string descriptor within the device.
 *  This unique serial number is used by the host to associate resources to the device (such as drivers or COM port
 *  number allocations) to a device regardless of the port it is plugged in to on the host. Some microcontrollers contain
 *  a unique serial number internally, and setting the device descriptors serial number string index to this value
 *  will cause it to use the internal serial number.
 *
 *  On unsupported devices, this will evaluate to \ref NO_DESCRIPTOR and so will force the host to create a pseudo-serial
 *  number for the device.
 */
#define USE_INTERNAL_SERIAL            0xDC

/** Length of the device's unique internal serial number, in bits, if present on the selected microcontroller
 *  model.
 */
#define INTERNAL_SERIAL_LENGTH_BITS    (8 * (1 + (offsetof(NVM_PROD_SIGNATURES_t, COORDY1) - offsetof(NVM_PROD_SIGNATURES_t, LOTNUM0))))

/** Start address of the internal serial number, in the appropriate address space, if present on the selected microcontroller
 *  model.
 */
#define INTERNAL_SERIAL_START_ADDRESS  offsetof(NVM_PROD_SIGNATURES_t, LOTNUM0)

/* Enums: */
	/** Enum for the various states of the USB Device state machine. Only some states are
	 *  implemented in the LUFA library - other states are left to the user to implement.
	 *
	 *  For information on each possible USB device state, refer to the USB 2.0 specification.
	 *
	 *  \see \ref USB_DeviceState, which stores the current device state machine state.
	 */
	enum USB_Device_States_t
	{
		DEVICE_STATE_Unattached                   = 0, /**< Internally implemented by the library. This state indicates
		                                                *   that the device is not currently connected to a host.
		                                                */
		DEVICE_STATE_Powered                      = 1, /**< Internally implemented by the library. This state indicates
		                                                *   that the device is connected to a host, but enumeration has not
		                                                *   yet begun.
		                                                */
		DEVICE_STATE_Default                      = 2, /**< Internally implemented by the library. This state indicates
		                                                *   that the device's USB bus has been reset by the host and it is
		                                                *   now waiting for the host to begin the enumeration process.
		                                                */
		DEVICE_STATE_Addressed                    = 3, /**< Internally implemented by the library. This state indicates
		                                                *   that the device has been addressed by the USB Host, but is not
		                                                *   yet configured.
		                                                */
		DEVICE_STATE_Configured                   = 4, /**< May be implemented by the user project. This state indicates
		                                                *   that the device has been enumerated by the host and is ready
		                                                *   for USB communications to begin.
		                                                */
		DEVICE_STATE_Suspended                    = 5, /**< May be implemented by the user project. This state indicates
		                                                *   that the USB bus has been suspended by the host, and the device
		                                                *   should power down to a minimal power level until the bus is
		                                                *   resumed.
		                                                */
	};

#define USB_EP_size_to_gc(x)  ((x <= 8   )?USB_EP_BUFSIZE_8_gc:\
                               (x <= 16  )?USB_EP_BUFSIZE_16_gc:\
                               (x <= 32  )?USB_EP_BUFSIZE_32_gc:\
                               (x <= 64  )?USB_EP_BUFSIZE_64_gc:\
                               (x <= 128 )?USB_EP_BUFSIZE_128_gc:\
                               (x <= 256 )?USB_EP_BUFSIZE_256_gc:\
                               (x <= 512 )?USB_EP_BUFSIZE_512_gc:\
                                           USB_EP_BUFSIZE_1023_gc)

#define USB_EP_IN 0x80

// Flag in the endpoint address to indicate that the endpoint should use
// PingPong (double buffer) mode. This is not actually part of the endpoint
// address as seen by the host. If PP is enabled, this flag needs to be part
// of the address passed to all USB_EP_* functions.
#define USB_EP_PP 0x40
	
extern volatile uint8_t USB_DeviceState;
extern volatile uint8_t USB_Device_ConfigurationNumber;

/** Configure the XMEGA's clock for use with USB.  */
void USB_ConfigureClock(void);

/** Initialize USB functionality */
void USB_Init(void);
void USB_ResetInterface(void);

#define _USB_EP(epaddr) \
	USB_EP_pair_t* pair = &endpoints[(epaddr & 0x3F)]; \
	USB_EP_t* e __attribute__ ((unused)) = &pair->ep[!!(epaddr&0x80)]; \
	
#define _USB_EP_OTHER(epaddr) \
	USB_EP_t* other = &pair->ep[!(epaddr&0x80)]
	
#define _USB_EP_BANK(epaddr, bank) \
	USB_EP_t* b = &pair->ep[!!(epaddr&0x80) != bank]

inline void USB_ep_init(uint8_t ep, uint8_t type, uint16_t bufsize) ATTR_ALWAYS_INLINE;
inline void USB_ep_init(uint8_t ep, uint8_t type, uint16_t bufsize){
	_USB_EP(ep);
	if (ep & USB_EP_PP){
		_USB_EP_OTHER(ep);
		e->STATUS = USB_EP_BUSNACK0_bm | USB_EP_BUSNACK1_bm;
		e->CTRL = type | USB_EP_size_to_gc(bufsize) | USB_EP_PINGPONG_bm;
		other->CTRL = 0;
		other->STATUS = USB_EP_BUSNACK0_bm;
	}else{
		e->STATUS = USB_EP_BUSNACK0_bm;
		e->CTRL = type | USB_EP_size_to_gc(bufsize);
	}
}

inline void USB_ep_cancel(uint8_t ep) ATTR_ALWAYS_INLINE;
inline void USB_ep_cancel(uint8_t ep){
	_USB_EP(ep);
	if (ep & USB_EP_PP){
		LASR16(&e->STATUS, USB_EP_BUSNACK0_bm | USB_EP_BUSNACK1_bm);
		LACR16(&e->STATUS, USB_EP_BANK_bm);
	}else{
		LASR16(&e->STATUS, USB_EP_BUSNACK0_bm);
	}
}

inline void USB_ep_start_bank(uint8_t ep, uint8_t bank, uint8_t* addr, uint16_t size){
	_USB_EP(ep);
	_USB_EP_BANK(ep, bank);
	b->DATAPTR = (unsigned) addr;
	if (ep & USB_EP_IN) b->CNT = size;
	
	//TODO: the OVF, STALL, and TRNCOMPL flags are in b->STATUS. Clear them if anyone cares.

	if (bank==0){
		LACR16(&(e->STATUS), USB_EP_BUSNACK0_bm | USB_EP_TRNCOMPL0_bm);
	}else{
		LACR16(&(e->STATUS), USB_EP_BUSNACK1_bm | USB_EP_TRNCOMPL1_bm);
	}
}

inline void USB_ep_out_start(uint8_t ep, uint8_t* addr) ATTR_ALWAYS_INLINE;
inline void USB_ep_out_start(uint8_t ep, uint8_t* addr){
	USB_ep_start_bank(ep, 0, addr, 0);
}

inline void USB_ep_in_start(uint8_t ep, const uint8_t* addr, uint16_t size) ATTR_ALWAYS_INLINE;
inline void USB_ep_in_start(uint8_t ep, const uint8_t* addr, uint16_t size){
	USB_ep_start_bank(ep, 0, (uint8_t*) addr, size);
}

inline bool USB_ep_done_bank(uint8_t ep, uint8_t bank){
	_USB_EP(ep);
	return e->STATUS & (bank?USB_EP_TRNCOMPL1_bm:USB_EP_TRNCOMPL0_bm);
}

inline bool USB_ep_done(uint8_t ep) ATTR_ALWAYS_INLINE;
inline bool USB_ep_done(uint8_t ep){
	_USB_EP(ep);
	if (ep & USB_EP_PP){
		return e->STATUS & (USB_EP_TRNCOMPL0_bm|USB_EP_TRNCOMPL1_bm);
	}else{
		// Because for ep0, TRNCOMPL1 is SETUP
		return e->STATUS & USB_EP_TRNCOMPL0_bm;
	}
}

inline void USB_ep_clear_done(uint8_t ep) ATTR_ALWAYS_INLINE;
inline void USB_ep_clear_done(uint8_t ep){
	_USB_EP(ep);
	if (ep & USB_EP_PP){
		LACR16(&(e->STATUS), USB_EP_TRNCOMPL0_bm|USB_EP_TRNCOMPL1_bm);
	}else{
		// Because for ep0, TRNCOMPL1 is SETUP
		LACR16(&(e->STATUS), USB_EP_TRNCOMPL0_bm);
	}
}

inline bool USB_ep_ready(uint8_t ep) ATTR_ALWAYS_INLINE;
inline bool USB_ep_ready(uint8_t ep){
	_USB_EP(ep);
	return e->STATUS & (USB_EP_BUSNACK0_bm | USB_EP_BUSNACK1_bm);
}

inline bool USB_ep_empty(uint8_t ep) ATTR_ALWAYS_INLINE;
inline bool USB_ep_empty(uint8_t ep){
	_USB_EP(ep);
	if (ep & USB_EP_PP){
		const uint8_t mask = (USB_EP_BUSNACK0_bm | USB_EP_BUSNACK1_bm);
		return (e->STATUS & mask) == mask;
	}else{
		return e->STATUS & USB_EP_BUSNACK0_bm;
	}
}

inline uint16_t USB_ep_count_bank(uint8_t ep, uint8_t bank) ATTR_ALWAYS_INLINE;
inline uint16_t USB_ep_count_bank(uint8_t ep, uint8_t bank){
	_USB_EP(ep);
	_USB_EP_BANK(ep, bank);
	return b->CNT;
}

inline uint16_t USB_ep_count(uint8_t ep) ATTR_ALWAYS_INLINE;
inline uint16_t USB_ep_count(uint8_t ep){
	return USB_ep_count_bank(ep, 0);
}

inline void USB_ep0_send(uint8_t size){
	USB_ep_in_start(0x80, ep0_buf_in, size);
}

const uint8_t* USB_ep0_from_progmem(const uint8_t* addr, uint16_t size);

inline void USB_ep_wait(uint8_t ep) ATTR_ALWAYS_INLINE;
inline void USB_ep_wait(uint8_t ep){
	while (!USB_ep_done(ep)){};
}

/// Select a certain bank of an endpoint for the next transfer
inline void USB_ep_set_bank(uint8_t ep, uint8_t bank) ATTR_ALWAYS_INLINE;
inline void USB_ep_set_bank(uint8_t ep, uint8_t bank){
	_USB_EP(ep);
	if (bank){
		LASR16(&(e->STATUS), USB_EP_BANK_bm);
	}else{
		LACR16(&(e->STATUS), USB_EP_BANK_bm);
	}
}

/// Get the bank 0/1 which will handle the next request on this endpoint
inline uint8_t USB_ep_get_bank(uint8_t ep) ATTR_ALWAYS_INLINE;
inline uint8_t USB_ep_get_bank(uint8_t ep){
	_USB_EP(ep);
	return !!(e->STATUS & USB_EP_BANK_bm);
}


/// Enable the OUT stage on the default control pipe. 
inline void USB_ep0_enableOut(void) ATTR_ALWAYS_INLINE;
inline void USB_ep0_enableOut(void){
	LACR16(&endpoints[0].out.STATUS, USB_EP_SETUP_bm | USB_EP_BUSNACK0_bm | USB_EP_TRNCOMPL0_bm | USB_EP_OVF_bm);
}

/** Detaches the device from the USB bus. This has the effect of removing the device from any
 *  attached host, ceasing USB communications. If no host is present, this prevents any host from
 *  enumerating the device once attached until \ref USB_Attach() is called.
 */
static inline void USB_Detach(void) ATTR_ALWAYS_INLINE;
static inline void USB_Detach(void)
{
	USB.CTRLB &= ~USB_ATTACH_bm;
}

/** Attaches the device to the USB bus. This announces the device's presence to any attached
 *  USB host, starting the enumeration process. If no host is present, attaching the device
 *  will allow for enumeration once a host is connected to the device.
 */
static inline void USB_Attach(void) ATTR_ALWAYS_INLINE;
static inline void USB_Attach(void)
{
	USB.CTRLB |= USB_ATTACH_bm;
}

inline void USB_ep0_stall(void) {
	endpoints[0].out.CTRL |= USB_EP_STALL_bm;
	endpoints[0].in.CTRL  |= USB_EP_STALL_bm;
}

static inline void USB_enter_bootloader(void){
	cli();
	USB_ep0_send(0);
	USB_ep0_enableOut();
	USB_ep_wait(0x00); // Wait for the status stage to complete
	_delay_ms(10);
	USB_Detach();
	_delay_ms(100);
	void (*enter_bootloader)(void) = (void*) 0x47fc /*0x8ff8/2*/;
	enter_bootloader();
}