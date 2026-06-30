#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
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

#define REQUEST_INDEX_EP1                   (REQUEST_INDEX_ENDPOINT_DIRECTION_IN | 1)

static USBDeviceState gUSBState = USBDeviceState::NONE;
static USBDeviceState gUSBStateBeforeSuspend = USBDeviceState::NONE;
static u8 gSelectedConfiguration = 0;
static u8 gAddressPending = 0;
static u8 gWasReset = 0;
static u16 gHIDIdleTime = 0; // time in ms to repeat unchanged HID report

enum class RequestProcessResult : u8
{
    HANDLED,
    NOT_HANDLED,
    ERROR
};

/* Unrecoverable error, will halt all further execution */
void USBError(u8 err)
{
    cli();

    if (gUSBState == USBDeviceState::ERROR)
    {
        return;
    }
    gUSBState = USBDeviceState::ERROR;
    DEBUG_OUT(err, DEBUG_PRIORITY_ERROR, 1);
    Led1On();
    Led2On();
    
    sleep_mode(); // interrupts are disabled, will sleep until hardware reset
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

static void USBDeviceUpdate(void)
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

static RequestProcessResult USBProcessStandardRequest(const USBSetupRequest& request)
{
    RequestProcessResult result = RequestProcessResult::NOT_HANDLED;

    EP_CLEAR_SETUP_RECEIVED();

    switch (request.code)
    {
        case (u8)SetupRequestCode::GET_STATUS:
        {
            // DIRECTION: to host
            // RECEPIENT: device, interface, endpoint
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::IN)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (request.dataSize != 2)                  { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (request.value != 0)                     { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (gUSBState <= USBDeviceState::DEFAULT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_DEVICE)
            {
                EP0_Send16(0);
                result = RequestProcessResult::HANDLED;
            }
            else if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_INTERFACE)
            {
                EP0_Send16(0);
                result = RequestProcessResult::HANDLED;
            }
            else if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_ENDPOINT && request.index == REQUEST_INDEX_EP1)
            {
                u16 epStatus = EP1_IsHalted() ? 1 : 0;
                EP0_Send16(epStatus);
                result = RequestProcessResult::HANDLED;
            }

        } break;

        case (u8)SetupRequestCode::CLEAR_FEATURE:
        {
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::OUT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (request.dataSize != 0)                  { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (gUSBState <= USBDeviceState::DEFAULT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_ENDPOINT && gUSBState == USBDeviceState::CONFIGURED && request.index == REQUEST_INDEX_EP1)
            {
                if (request.value == (u16)StandardFeature::ENDPOINT_HALT)
                {
                    EP1_ClearHalt();
                    result = RequestProcessResult::HANDLED;
                }
            }
        } break;

        case (u8)SetupRequestCode::SET_FEATURE:
        {
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::OUT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (request.dataSize != 0)                                                  { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            if ((request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_ENDPOINT && gUSBState == USBDeviceState::CONFIGURED && request.index == REQUEST_INDEX_EP1)
            {
                if (request.value == (u16)StandardFeature::ENDPOINT_HALT)
                {
                    EP1_SetHalt();
                    result = RequestProcessResult::HANDLED;
                }
            }
        } break;

        case (u8)SetupRequestCode::SET_ADDRESS:
        {
            // value: device address
            // length: 0
            // DIRECTION: no data
            // RECEPIENT: device
            // The USB device does not change its device address until after the Status stage of this request is completed successfully.
            //  Note that this is a difference between this request and all other requests. For all other requests, the operation indicated must be completed before the Status stage.
            DEBUG_OUT(DEBUG_STAGE_SET_ADDRESS);

            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::OUT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (request.dataSize != 0)                                                  { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (request.index != 0)                                                     { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (request.value > 127)                                                    { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            if (gUSBState == USBDeviceState::CONFIGURED)                                { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            u8 address = (request.value & 0x7F);
            UDADDR = address;
            gAddressPending = 1;
            result = RequestProcessResult::HANDLED;
        } break;

        case (u8)SetupRequestCode::GET_DESCRIPTOR:
        {
            // value: descriptor type and index
            // length: descriptor length
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::IN)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            u8 descriptorType = (request.value >> 8) & 0xff;
            u8 descriptorIndex = request.value & 0xff;
            u16 language = request.index;

            u16 descriptorSize = 0;
            const u8* descriptorAddr = 0;
            if (descriptorType == (u8)DescriptorType::DEVICE)
            {
                descriptorSize = sizeof(DeviceDescriptor);
                descriptorAddr = (u8*)&deviceDescriptor;
            }
            else if (descriptorType == (u8)DescriptorType::CONFIGURATION)
            {
                descriptorSize =  sizeof(Configuration);
                descriptorAddr = (u8*)(&configuration);
            }
            else if (descriptorType == (u8)DescriptorType::HID)
            {
                descriptorSize =  sizeof(HIDDescriptor);
                descriptorAddr = (u8*)(&configuration.hidDescriptor);
            }
            else if (descriptorType == (u8)DescriptorType::HID_REPORT)
            {
                descriptorSize = sizeof(hidReportDescriptor);
                descriptorAddr = hidReportDescriptor;
            }

            if (descriptorSize > 0 && descriptorAddr)
            {
                const u16 bytesToSend = Min(request.dataSize, descriptorSize);
                EP0_SendBufferPgm(descriptorAddr, bytesToSend);
                if (bytesToSend < request.dataSize && bytesToSend % 8 == 0)
                {
                    // Send ZLP if sending fewer bytes than requested and all packets were full
                    while(!EP_IS_IN_BANK_READY());
                    EP0_SEND_IN_PACKET();
                }
                result = RequestProcessResult::HANDLED;
            }
        } break;

        case (u8)SetupRequestCode::SET_DESCRIPTOR:
        {
            // value: descriptor type and index
            // length: descriptor length
            // DIRECTION: to device
            // RECEPIENT: device
        } break;

        case (u8)SetupRequestCode::GET_CONFIGURATION:
        {
            // value: 0
            // length: 1
            // DIRECTION: to host
            // RECEPIENT: device
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::IN)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            if (gUSBState == USBDeviceState::ADDRESSED)
            {
                EP0_Send8(gSelectedConfiguration);
                result = RequestProcessResult::HANDLED;
            }
            else if (gUSBState == USBDeviceState::CONFIGURED)
            {
                EP0_Send8(gSelectedConfiguration);
                result = RequestProcessResult::HANDLED;
            }
        } break;

        case (u8)SetupRequestCode::SET_CONFIGURATION:
        {
            // value: configuration value
            // length: 0
            // DIRECTION: no data
            // RECEPIENT: device
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::OUT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            if (!EP1_Setup())       { USBError(DEBUG_ERROR_EP_SETUP); return RequestProcessResult::ERROR; }
            if (!IsEP1Configured()) { USBError(DEBUG_ERROR_EP_SETUP); return RequestProcessResult:: ERROR; }
            
            gSelectedConfiguration = (u8)request.value;
            result = RequestProcessResult::HANDLED;
            EP1_ClearHalt();
            gUSBState = USBDeviceState::CONFIGURED;
            TC0_INTERRUPTA_DISABLE();
            USB_INT_SOF_CLEAR();
            USB_INT_SOF_ENABLE();
            DEBUG_OUT(DEBUG_STAGE_CONFIGURED);
        } break;

        case (u8)SetupRequestCode::GET_INTERFACE:
        {
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::IN)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            // value: configuration value
            // length: 1
            // DIRECTION: to host
            // RECEPIENT: interface
        } break;

        case (u8)SetupRequestCode::SET_INTERFACE:
        {
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::OUT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            // value: alternate setting
            // length: 0
            // DIRECTION: no data
            // RECEPIENT: interface
            // TODO: clear HALT
        } break;

        case (u8)SetupRequestCode::SYNCH_FRAME:
        {
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::OUT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            // value: configuration value
            // length: 0
            // DIRECTION: no data
            // RECEPIENT: device
        } break;

        default:
        {
            USBError(DEBUG_ERROR_UNKNOWN_REQUEST);
            result = RequestProcessResult::ERROR;
        } break;
    }

    return result;
}

static RequestProcessResult USBProcessHIDRequest(const USBSetupRequest& request)
{
    RequestProcessResult result = RequestProcessResult::NOT_HANDLED;
    EP_CLEAR_SETUP_RECEIVED();

    switch (request.code)
    {
        case (u8)HIDRequestCode::GET_REPORT:
        {
            u8 reportType = request.value >> 8;
            u8 reportId = request.value & 0xff;
            Report report;
            report.id = 1;
            report.brake = 0;
            EP0_SendBuffer((u8*)&report, sizeof(Report));

            result = RequestProcessResult::HANDLED;
        } break;

        case (u8)HIDRequestCode::GET_IDLE:
        {
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::IN)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }
            EP0_Send8(gHIDIdleTime >> 2);

            result = RequestProcessResult::HANDLED;
        } break;

        case (u8)HIDRequestCode::SET_IDLE:
        {
            if (GET_REQUEST_DATA_DIRECTION(request.attributes) != DataDirection::OUT)   { USBError(DEBUG_ERROR_INVALID_REQUEST); return RequestProcessResult::ERROR; }

            u8 duration = request.value >> 8;
            u8 reportId = request.value & 0xff;
            gHIDIdleTime = duration << 2;

            result = RequestProcessResult::HANDLED;
        } break;

        case (u8)HIDRequestCode::SET_REPORT:
        case (u8)HIDRequestCode::GET_PROTOCOL:
        case (u8)HIDRequestCode::SET_PROTOCOL:
        default:
        {
        } break;
    }

    return result;
}

static void USBProcessSetupRequest()
{
	EP_SELECT(0);
    EP_STALL_DISABLE();
    gWasReset = false;

    USBSetupRequest request = {0};
    u8* write = (u8*)(&request);
    // TODO: check that the FIFO contains the expected number of bytes
    static_assert(sizeof(USBSetupRequest) == 8, "USBSetupRequest must be 8 bytes");
	for (u8 bytesRead = 0; bytesRead < sizeof(USBSetupRequest); ++bytesRead)
	{
		*(write++) = EP_READ_BYTE();
	}

    RequestProcessResult result = RequestProcessResult::NOT_HANDLED;

    if ((request.attributes & REQUEST_ATTR_TYPE_MASK) == REQUEST_ATTR_TYPE_STANDARD)
    {
        result = USBProcessStandardRequest(request);
    }
    else if ((request.attributes & REQUEST_ATTR_TYPE_MASK) == REQUEST_ATTR_TYPE_CLASS 
             && (request.attributes & REQUEST_ATTR_RECEPIENT_MASK) == REQUEST_ATTR_RECEPIENT_INTERFACE)
    {
        result = USBProcessHIDRequest(request);
    }
    else
    {
        EP_CLEAR_SETUP_RECEIVED();
    }
    
    if (result == RequestProcessResult::ERROR)
    {
        return;
    }

    if (result == RequestProcessResult::NOT_HANDLED)
    {
        EP_STALL_ENABLE();
        return;
    }

    if (GET_REQUEST_DATA_DIRECTION(request.attributes) == DataDirection::OUT)
    {
        while (!EP_IS_IN_BANK_READY())
        {
            if (gWasReset)
            {
                return;
            }
            if (gUSBState <= USBDeviceState::POWERED)
            {
                return;
            }
        }
        EP0_SEND_IN_PACKET();
    }
    else
    {
        while (!EP_IS_OUT_PACKET_RECEIVED())
        {
            if (gWasReset)
            {
                return;
            }
            if (gUSBState <= USBDeviceState::POWERED)
            {
                return;
            }
        }
        EP0_ACK_OUT_PACKET();
    }

    if (gAddressPending)
    {
        gAddressPending = 0;
        if (UDADDR > 0)
        {
            while (!EP_IS_IN_BANK_READY());
            UDADDR |= _BV(ADDEN); // enable new address
            gUSBState = USBDeviceState::ADDRESSED;
            DEBUG_OUT(DEBUG_STAGE_ADDRESSED);
        }
        else
        {
            gUSBState = USBDeviceState::DEFAULT;
            DEBUG_OUT(DEBUG_STAGE_DEFAULT);
        }
    }
}

ISR(USB_GEN_vect, ISR_BLOCK)
{
    if (USB_INT_RESET_TRIGGERED())
    {
        USB_INT_RESET_CLEAR();
        if (gUSBState >= USBDeviceState::POWERED)
        {
            gWasReset = 1;
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
            return;
        }
    }

	if (USB_INT_SOF_TRIGGERED())
	{
        USB_INT_SOF_CLEAR();
        TickAsync();
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
            DEBUG_OUT(DEBUG_STAGE_ATTACHED);

            return;
		}
	}

	if (USB_INT_SUSPEND_TRIGGERED())
	{
        USB_INT_SUSPEND_CLEAR();

        #if 0
        if (gUSBState == USBDeviceState::SUSPENDED)
        {
            return;
        }

        Led1On();
        Led2Blink();

        USB_INT_SUSPEND_DISABLE();
        USB_INT_WAKE_UP_ENABLE();
        DEBUG_OUT(DEBUG_STAGE_SUSPENDED, true);

        gUSBStateBeforeSuspend = gUSBState;
		gUSBState = USBDeviceState::SUSPENDED;

		USBCON |= _BV(FRZCLK);
		USB_PLL_OFF();
        #endif
	}

	if (USB_INT_WAKE_UP_TRIGGERED())
	{
        USB_INT_WAKE_UP_CLEAR();
        #if 0
        if (gUSBState != USBDeviceState::SUSPENDED)
        {
            return;
        }

        Led1Off();
        Led2Blink();

        USB_INT_WAKE_UP_DISABLE();
        USB_INT_SUSPEND_ENABLE();

		gUSBState = gUSBStateBeforeSuspend;

		USB_PLL_ON();
		while (!IS_PLL_LOCKED());
		USBCON &= ~_BV(FRZCLK);
        #endif
	}
}