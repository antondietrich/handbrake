#pragma once

#define EP_SELECT(idx)                  UENUM = ((idx) & 0b111)
#define EP_CURRENT()                    (UENUM & 0b111)
#define EP_ENABLE()                     UECONX |= _BV(EPEN)
#define EP_IS_CONFIG_OK()               (UESTA0X & _BV(CFGOK))

#define EP_IS_IN_BANK_READY()           (UEINTX & _BV(TXINI))
//#define EP_SEND_IN_PACKET()             UEINTX &= ~(_BV(TXINI) | _BV(FIFOCON))
#define EP_SEND_IN_PACKET()             UEINTX &= ~_BV(TXINI); UEINTX &= ~_BV(FIFOCON)
#define EP0_SEND_IN_PACKET()            UEINTX &= ~_BV(TXINI)
#define EP0_FIFO_SIZE_BYTES             8

#define EP_IS_OUT_PACKET_RECEIVED()     (UEINTX & _BV(RXOUTI))
#define EP_ACK_OUT_PACKET()             UEINTX &= ~(_BV(RXOUTI) | _BV(FIFOCON))
#define EP0_ACK_OUT_PACKET()            UEINTX &= ~_BV(RXOUTI)
#define EP_STALL_ENABLE()               UECONX |= _BV(STALLRQ)
#define EP_STALL_DISABLE()              UECONX |= _BV(STALLRQC)
#define EP_IS_STALL_ENABLED()           (UECONX & _BV(STALLRQ))
#define EP_RESET_DATA_TOGGLE()          UECONX |= _BV(RSTDT)

#define EP_TYPE_CONTROL                 0x0
#define EP_TYPE_INTERRUPT               _BV(EPTYPE0) | _BV(EPTYPE1)
#define EP_DIR_OUT                      0
#define EP_DIR_IN                       1

#define EP_READ_BYTE()                  (UEDATX)
#define EP_WRITE_BYTE(b)                UEDATX = (b)

// enable Setup Received interrupt
//UEIENX |= (1 << RXSTPE);
#define EP_IS_SETUP_RECEIVED()          (UEINTX & _BV(RXSTPI))
/* This also handshakes the request */
#define EP_CLEAR_SETUP_RECEIVED()       UEINTX &= ~_BV(RXSTPI)

#define EP_IS_INTERRUPT_ON_EP0          (UEINT & _BV(EPINT0))
#define EP_IS_INTERRUPT_ON_EP1          (UEINT & _BV(EPINT1))


bool IsEP0Configured();
bool EP0_Setup();
void EP0_Send8(u8 value);
void EP0_Send16(u16 value);
// Send a buffer in RAM
void EP0_SendBuffer(const u8* buffer, u16 size);
// Send a buffer in PROGMEM
void EP0_SendBufferPgm(const u8* buffer, u16 size);

bool EP1_Setup();
bool IsEP1Configured();
void EP_Send8(u8 value);
void EP_SendBuffer(const u8* buffer, u16 size);

void EP1_SetHalt();
bool EP1_GetHalt();
void EP1_ClearHalt();