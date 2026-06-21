#pragma once

#define EP_SELECT(idx)                  UENUM = ((idx) & 0b111)
#define EP_ENABLE()                     UECONX |= _BV(EPEN)
#define EP_IS_CONFIG_OK()               (UESTA0X & _BV(CFGOK))

#define EP_IS_IN_BANK_READY()           (UEINTX & _BV(TXINI))
#define EP_SEND_IN_PACKET()             UEINTX &= ~(_BV(TXINI) | _BV(FIFOCON))
#define EP0_SEND_IN_PACKET()            UEINTX &= ~_BV(TXINI)
#define EP0_FIFO_SIZE_BYTES             8

#define EP_IS_OUT_PACKET_RECEIVED()     (UEINTX & _BV(RXOUTI))
#define EP_ACK_OUT_PACKET()             UEINTX &= ~(_BV(RXOUTI) | _BV(FIFOCON))
#define EP0_ACK_OUT_PACKET()            UEINTX &= ~_BV(RXOUTI);

#define EP_TYPE_CONTROL                 0x0
#define EP_TYPE_INTERRUPT               _BV(EPTYPE0) | _BV(EPTYPE1)

#define EP_READ_BYTE()                  (UEDATX)
#define EP_WRITE_BYTE(b)                UEDATX = (b)

// enable Setup Received interrupt
//UEIENX |= (1 << RXSTPE);
#define EP_IS_SETUP_RECEIVED()          (UEINTX & _BV(RXSTPI))
#define EP_CLEAR_SETUP_RECEIVED()       UEINTX &= ~_BV(RXSTPI)

#define EP_IS_INTERRUPT_ON_EP0          (UEINT & _BV(EPINT0))
#define EP_IS_INTERRUPT_ON_EP1          (UEINT & _BV(EPINT1))

bool IsEP0Configured();
bool EP0_Setup();
void EP0_Send8(u8 value);
void EP0_Send16(u16 value);
// Send a buffer in RAM
void EP0_SendBuffer(u8* buffer, u16 size);
// Send a buffer in PROGMEM
void EP0_SendBufferPgm(u8* buffer, u16 size);