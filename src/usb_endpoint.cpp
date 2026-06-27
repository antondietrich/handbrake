#include <avr/io.h>
#include <avr/pgmspace.h>
#include "usb_endpoint.h"
#include "avr_common.h"


static u8 gEP1Halt = 0;

bool EP0_Setup()
{
	//Endpoint_ConfigureEndpoint(ENDPOINT_CONTROLEP, EP_TYPE_CONTROL, USB_Device_ControlEndpointSize, 1);

    /*
    The FIFOCON and RWAL fields are irrelevant with CONTROL endpoints
    CONTROL endpoints are managed by the following bits:
    > RXSTPI is set when a new SETUP is received. It shall be cleared by firmware to acknowledge the packet
    and to clear the endpoint bank.
    > RXOUTI is set when a new OUT data is received. It shall be cleared by firmware to acknowledge the
    packet and to clear the endpoint bank.
    > TXINI is set when the bank is ready to accept a new IN packet. It shall be cleared by firmware to send the
    packet and to clear the endpoint bank.
    */
    EP_SELECT(0);
    //UERST |= _BV(EPRST0); // reset EP0
    //UERST &= ~_BV(EPRST0);

    EP_ENABLE();
    //ACTIVATE_EP(); EP0 always active
    UECFG0X = 0; // control EP
    UECFG1X = _BV(ALLOC); // 8 bytes, single bank, allocate
    UEIENX = 0;

    if (EP_IS_CONFIG_OK())
    {
        return true;
    }

    return false;
}

bool IsEP0Configured()
{
    EP_SELECT(0);
    if (!(UECONX & _BV(EPEN))) return false;
    if (!(UECFG1X & _BV(ALLOC))) return false;
    if (UECFG0X != 0) return false;
    if (!EP_IS_CONFIG_OK()) return false;
    return true;
}

void EP0_Send8(u8 value)
{
    while(!EP_IS_IN_BANK_READY());

    EP_WRITE_BYTE(value);

    EP0_SEND_IN_PACKET();
}

void EP0_Send16(u16 value)
{
    while(!EP_IS_IN_BANK_READY());

    EP_WRITE_BYTE(value & 0xff);
    EP_WRITE_BYTE((value >> 8) & 0xff);

    EP0_SEND_IN_PACKET();
}

void EP0_SendBuffer(const u8* buffer, u16 size)
{
    const u8* read = buffer;
    while (size > 0)
    {
        while(!EP_IS_IN_BANK_READY());

        u8 bytesWritten = 0;
        while (size > 0 && bytesWritten < EP0_FIFO_SIZE_BYTES)
        {
            EP_WRITE_BYTE(*read++);
            ++bytesWritten;
            --size;
        }

        EP0_SEND_IN_PACKET();
    }
}

void EP0_SendBufferPgm(const u8* buffer, u16 size)
{
    while (size > 0)
    {
        while(!EP_IS_IN_BANK_READY());

        u8 bytesWritten = 0;
        while (size > 0 && bytesWritten < EP0_FIFO_SIZE_BYTES)
        {
            u8 byte = pgm_read_byte(buffer++);
            EP_WRITE_BYTE(byte);
            ++bytesWritten;
            --size;
        }

        EP0_SEND_IN_PACKET();
    }
}

bool EP1_Setup()
{
    EP_SELECT(1);
    EP_ENABLE();

    UERST |= _BV(EPRST1);
    UERST &= ~_BV(EPRST1);

    //UECFG0X = (EP_TYPE_INTERRUPT | EP_DIR_IN);
    UECFG0X = 0b11000001u;
    UECFG1X = _BV(ALLOC);
    UEIENX = 0;

    EP_STALL_DISABLE();
    EP_RESET_DATA_TOGGLE();

    if (EP_IS_CONFIG_OK())
    {
        return true;
    }

    return false;
}

void EP_Send8(u8 value)
{
    while(!EP_IS_IN_BANK_READY());

    EP_WRITE_BYTE(value);

    EP_SEND_IN_PACKET();
}

void EP_SendBuffer(const u8* buffer, u16 size)
{
    const u8* read = buffer;
    while (size > 0)
    {
        while(!EP_IS_IN_BANK_READY());

        u8 bytesWritten = 0;
        while (size > 0 && bytesWritten < EP0_FIFO_SIZE_BYTES)
        {
            EP_WRITE_BYTE(*read++);
            ++bytesWritten;
            --size;
        }

        EP_SEND_IN_PACKET();
    }
}

bool IsEP1Configured()
{
    EP_SELECT(1);
    if (!(UECONX & _BV(EPEN))) return false;
    if (!(UECFG1X & _BV(ALLOC))) return false;
    if (UECFG0X != (EP_TYPE_INTERRUPT | EP_DIR_IN)) return false;
    if (!EP_IS_CONFIG_OK()) return false;
    return true;
}

void EP1_SetHalt()
{
    gEP1Halt = 1;
    u8 ep = UENUM;
    EP_SELECT(1);
    EP_STALL_ENABLE();
    EP_SELECT(ep);
}

bool EP1_IsHalted()
{
    return gEP1Halt == 1;
}

void EP1_ClearHalt()
{
    gEP1Halt = 0;
    u8 ep = UENUM;
    EP_SELECT(1);
    EP_STALL_DISABLE();
    EP_RESET_DATA_TOGGLE();
    EP_SELECT(ep);
}

// TODO EP Interrupt:
// read UEINT to determine EP
// poll flags to detect specific event