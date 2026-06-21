#include <avr/io.h>
#include <avr/interrupt.h>
#include "avr_common.h"
#include "avr_debug.h"
#include "usb_protocol.h"
#include "usb_endpoint.h"
#include "usb_device.h"
#include "descriptors.h"

/* Toggle host presence sensing pad */
#define VBUSPAD_ON()		USBCON |= _BV(OTGPADE)
#define VBUSPAD_OFF()		USBCON &= ~_BV(OTGPADE)

/* VBUS is set when the device is connected to USB host */
#define IS_VBUS_CONNECTED()		USBSTA & _BV(VBUS)

#define USB_ENABLE()        USBCON |= _BV(USBE)

/* Toggle internal USB frontend voltage regulator */
#define USB_VREG_ON()		UHWCON |=  _BV(UVREGE)
#define USB_VREG_OFF()		UHWCON &= ~_BV(UVREGE)

/* USB clock */
#define USB_PLL_ON()		PLLCSR = _BV(PINDIV); PLLCSR |= _BV(PLLE)
#define USB_PLL_OFF()		PLLCSR = 0
#define IS_PLL_LOCKED()		(PLLCSR & _BV(PLOCK))

/* Interrupts */
#define USB_INTERRUPTS_OFF()	            USBCON &= ~_BV(VBUSTE);    UDIEN = 0
#define USB_INTERRUPTS_CLEAR()	            USBINT = 0;                UDINT = 0

#define USB_INT_VBUS_CONNECT_ENABLE()       USBCON |= (1 << VBUSTE)
#define USB_INT_WAKE_UP_ENABLE()            UDIEN |= (1 << WAKEUPE)
#define USB_INT_SUSPEND_ENABLE()            UDIEN |= (1 << SUSPE)
#define USB_INT_RESET_ENABLE()              UDIEN |= (1 << EORSTE)
#define USB_INT_SOF_ENABLE()                UDIEN |= (1 << SOFE)

#define USB_INT_VBUS_CONNECT_DISABLE()      USBCON &= ~(1 << VBUSTE)
#define USB_INT_WAKE_UP_DISABLE()           UDIEN &= ~(1 << WAKEUPE)
#define USB_INT_SUSPEND_DISABLE()           UDIEN &= ~(1 << SUSPE)
#define USB_INT_RESET_DISABLE()             UDIEN &= ~(1 << EORSTE)
#define USB_INT_SOF_DISABLE()               UDIEN &= ~(1 << SOFE)

#define USB_INT_VBUS_CONNECT_CLEAR()        USBINT &= ~(1 << VBUSTI)
#define USB_INT_WAKE_UP_CLEAR()             UDINT &= ~(1 << WAKEUPI)
#define USB_INT_SUSPEND_CLEAR()             UDINT &= ~(1 << SUSPI)
#define USB_INT_RESET_CLEAR()               UDINT &= ~(1 << EORSTI)
#define USB_INT_SOF_CLEAR()                 UDINT &= ~(1 << SOFI)

#define USB_INT_VBUS_CONNECT_TRIGGERED()    (USBINT & (1 << VBUSTI))
#define USB_INT_WAKE_UP_TRIGGERED()         (UDINT & (1 << WAKEUPI))
#define USB_INT_SUSPEND_TRIGGERED()         (UDINT & (1 << SUSPI))
#define USB_INT_RESET_TRIGGERED()           (UDINT & (1 << EORSTI))
#define USB_INT_SOF_TRIGGERED()             (UDINT & (1 << SOFI))
	
static USBDeviceState gUSBState = USBDeviceState::NONE;
static USBDeviceState gUSBStateBeforeSuspend = USBDeviceState::NONE;

void USBError(u8 err)
{
    USB_INTERRUPTS_OFF();
    USB_INTERRUPTS_CLEAR();
    gUSBState = USBDeviceState::ERROR;
    DEBUG_OUT(err);
}

void USBInit()
{
	USB_VREG_ON();

	USB_INTERRUPTS_OFF();
	USB_INTERRUPTS_CLEAR();

	PLLFRQ = _BV(PDIV2); // 48MHz
	PLLCSR = 0;          // keep PLL disabled for now, we'll reenable it once VBUS is connected

    USB_PLL_ON();
    while (!IS_PLL_LOCKED());

    // unfreeze USB clock
    USBCON &= ~_BV(FRZCLK);

    USB_ENABLE();

    UDCON &= ~_BV(LSM); // set Full Speed

    gUSBState = USBDeviceState::ATTACHED;
    DEBUG_OUT(DEBUG_STAGE_ATTACHED);

    USB_INT_SUSPEND_CLEAR();
    USB_INT_SUSPEND_ENABLE();
    USB_INT_VBUS_CONNECT_ENABLE();
    USB_INT_RESET_ENABLE();

    VBUSPAD_ON();
}

static void USBProcessSetupRequest();

static void USB_DeviceTask(void)
{
	EP_SELECT(0);

	if (gUSBState < USBDeviceState::DEFAULT)
	{
        if (EP_IS_SETUP_RECEIVED())
        {
            USBError(DEBUG_ERROR_EARLY_RX);
        }
		return;
	}

	if (EP_IS_SETUP_RECEIVED())
	{
		USBProcessSetupRequest();
	}
}

static void USBProcessSetupRequest()
{
    cli();
    //DEBUG_OUT(DEBUG_STAGE_SETUP_REQUEST);
    USBSetupRequest request;
    u8* write = (u8*)(&request);
    // TODO: check that the FIFO contains the expected number of bytes
	for (u8 bytesRead = 0; bytesRead < sizeof(USBSetupRequest); ++bytesRead)
	{
		*(write++) = EP_READ_BYTE();
	}

    //DEBUG_OUT((u8)request.code, true, true);

    switch (request.code)
    {
        case SetupRequestCode::GET_STATUS:
        {
            // TODO: error handling
            EP_CLEAR_SETUP_RECEIVED();
            if (request.value == 0 && request.dataSize == 2)
            {
                if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_DEVICE)
                {
                    EP0_Send16(0);
                    while(!EP_IS_OUT_PACKET_RECEIVED());
                    EP0_ACK_OUT_PACKET();
                }
                else if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_INTERFACE)
                {
                    EP0_Send16(0);
                    while(!EP_IS_OUT_PACKET_RECEIVED());
                    EP0_ACK_OUT_PACKET();
                }
                else if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_ENDPOINT)
                {
                    EP0_Send16(0);
                    while(!EP_IS_OUT_PACKET_RECEIVED());
                    EP0_ACK_OUT_PACKET();
                }
            }

        } break;

        case SetupRequestCode::SET_ADDRESS:
        {
            DEBUG_OUT(DEBUG_STAGE_SET_ADDRESS);

            if (request.attributes == (0))
            {
                u8 address = (request.value & 0x7F);
                EP_CLEAR_SETUP_RECEIVED();

                if (gUSBState == USBDeviceState::DEFAULT || gUSBState == USBDeviceState::ADDRESSED)
                {
                    UDADDR = address;

                    // status transaction
                    while (!EP_IS_IN_BANK_READY());
                    EP0_SEND_IN_PACKET();
                    while (!EP_IS_IN_BANK_READY()); // wait for ACK

                    if (address > 0)
                    {
                        UDADDR |= _BV(ADDEN); // enable new address
                        gUSBState = USBDeviceState::ADDRESSED;
                        DEBUG_OUT(DEBUG_STAGE_ADDRESSED);
                    }
                    else
                    {
                        gUSBState = USBDeviceState::DEFAULT;
                    }
                }
                else if (gUSBState == USBDeviceState::CONFIGURED)
                {
                    gUSBState = USBDeviceState::ERROR;
                    USBError(DEBUG_ERROR_INVALID_ADDR);
                    return;
                }
            }
        } break;

        case SetupRequestCode::GET_DESCRIPTOR:
        {
            EP_CLEAR_SETUP_RECEIVED();
            DescriptorType descriptorType = (DescriptorType)((request.value >> 8) & 0xff);
            u8 descriptorIndex = request.value & 0xff;
            u16 language = request.index;

            // TODO: if request.dataSize > sizeof(Descriptor), send an empty packet in the end
            if (gUSBState == USBDeviceState::DEFAULT || gUSBState == USBDeviceState::ADDRESSED || gUSBState == USBDeviceState::CONFIGURED)
            {
                //DEBUG_OUT((u8)descriptorType, true, true);
                if (descriptorType == DescriptorType::DEVICE)
                {
                    const u16 bytesToSend = Min(request.dataSize, sizeof(DeviceDescriptor));
                    //DEBUG_OUT((u8)bytesToSend, true, true);
                    u8* buf = (u8*)(&deviceDescriptor);
                    //DEBUG_OUT(buf[3], true, true);
                    EP0_SendBufferPgm(buf, bytesToSend);
                    while(!EP_IS_OUT_PACKET_RECEIVED());
                    EP0_ACK_OUT_PACKET(); // status transaction
                }
                else if (descriptorType == DescriptorType::CONFIGURATION)
                {
                    const u16 bytesToSend = Min(request.dataSize, sizeof(Configuration));
                    EP0_SendBufferPgm((u8*)(&configuration), bytesToSend);
                    while(!EP_IS_OUT_PACKET_RECEIVED());
                    EP0_ACK_OUT_PACKET(); // status transaction
                }

            }
            else
            {
                gUSBState = USBDeviceState::ERROR;
                USBError(DEBUG_ERROR_INVALID_DESCR);
                return;
            }

        } break;

        case SetupRequestCode::SET_CONFIGURATION:
        {
            DEBUG_OUT(DEBUG_STAGE_CONFIGURED);

            gUSBState = USBDeviceState::CONFIGURED;
            EP_CLEAR_SETUP_RECEIVED();
            while (!EP_IS_IN_BANK_READY());
            EP0_SEND_IN_PACKET();
        } break;

        case SetupRequestCode::GET_CONFIGURATION:
        {
            EP_CLEAR_SETUP_RECEIVED();
            if (gUSBState == USBDeviceState::ADDRESSED)
            {
                EP0_Send8(0);
                while(!EP_IS_OUT_PACKET_RECEIVED());
                EP0_ACK_OUT_PACKET();
            }
            else if (gUSBState == USBDeviceState::CONFIGURED)
            {
                // TODO: send current configuration index
                EP0_Send8(1);
                while(!EP_IS_OUT_PACKET_RECEIVED());
                EP0_ACK_OUT_PACKET();
            }
        } break;

        default:
            break;
    }

    #if 0
	if (Endpoint_IsSETUPReceived())
	{
		Endpoint_ClearSETUP();
		Endpoint_StallTransaction();
	}
    #endif
    sei();
}

ISR(USB_GEN_vect, ISR_BLOCK)
{
    if (USB_INT_RESET_TRIGGERED())
    {
        USB_INT_RESET_CLEAR();
        if (gUSBState >= USBDeviceState::POWERED)
        {
            // EP0 should remain configured after reset
            gUSBState = USBDeviceState::DEFAULT;
            DEBUG_OUT(DEBUG_STAGE_DEFAULT);

            EP0_Setup();

            if (!(PLLCSR & _BV(PLOCK))) USBError(DEBUG_ERROR_PLLLOCK);
            if (!(USBCON & _BV(USBE))) USBError(DEBUG_ERROR_USBE);
            if ((USBCON & _BV(FRZCLK))) USBError(DEBUG_ERROR_FRZCLK);
            if ((UDCON & _BV(DETACH))) USBError(DEBUG_ERROR_ATTACH);
            if (!IsEP0Configured()) USBError(DEBUG_ERROR_EP_SETUP);
            // TODO: interrupt based comm?
            //USBInterruptEnable(USBInterrupt::RX_SETUP);
            return;
        }
    }

	if (USB_INT_SOF_TRIGGERED())
	{
        USB_INT_SOF_CLEAR();
	}

	if (USB_INT_VBUS_CONNECT_TRIGGERED())
	{
        USB_INT_VBUS_CONNECT_CLEAR();

		if (IS_VBUS_CONNECTED())
		{
            USB_INT_VBUS_CONNECT_CLEAR();
            USB_INT_RESET_ENABLE();

            gUSBState = USBDeviceState::POWERED;

            USBCON &= ~_BV(FRZCLK);

            if (!(PLLCSR & _BV(PLOCK))) USBError(DEBUG_ERROR_PLLLOCK);
            if (!(USBCON & _BV(USBE))) USBError(DEBUG_ERROR_USBE);
            if ((USBCON & _BV(FRZCLK))) USBError(DEBUG_ERROR_FRZCLK);

            if (!EP0_Setup())
            {
                USBError(DEBUG_ERROR_EP_SETUP);
                return;
            }

            UDCON &= ~_BV(DETACH); // Attach the device
            if ((UDCON & _BV(DETACH))) USBError(DEBUG_ERROR_ATTACH);
            DEBUG_OUT(DEBUG_STAGE_VBUS_DETECTED);

            return;
		}
		else
		{
            UDCON |= _BV(DETACH); // Detach the device
			gUSBState = USBDeviceState::ATTACHED;
            USB_INT_RESET_DISABLE();
            DEBUG_OUT(DEBUG_STAGE_ATTACHED, true);

            return;
		}
	}

	if (USB_INT_SUSPEND_TRIGGERED())
	{
        USB_INT_SUSPEND_CLEAR();

        if (gUSBState == USBDeviceState::SUSPENDED)
        {
            return;
        }

        USB_INT_SUSPEND_DISABLE();
        USB_INT_WAKE_UP_ENABLE();
        DEBUG_OUT(DEBUG_STAGE_SUSPENDED, true);

        gUSBStateBeforeSuspend = gUSBState;
		gUSBState = USBDeviceState::SUSPENDED;

		USBCON |= _BV(FRZCLK);
		USB_PLL_OFF();

        // TODO: mcu sleep mode
	}

	if (USB_INT_WAKE_UP_TRIGGERED())
	{
        USB_INT_WAKE_UP_CLEAR();
        if (gUSBState != USBDeviceState::SUSPENDED)
        {
            return;
        }

        USB_INT_WAKE_UP_DISABLE();
        USB_INT_SUSPEND_ENABLE();

		gUSBState = gUSBStateBeforeSuspend;

		USB_PLL_ON();
		while (!IS_PLL_LOCKED());

		USBCON &= ~_BV(FRZCLK);
	}
}