#ifndef AK_TM1637_H
#define AK_TM1637_H

#include "avr_common.h"
#include "ak_io.h"

#include "util/delay.h"

namespace TM1637
{

void Init();
void SetBrightness(u8 brightness);
void SetSegments(const uint8_t segments[], uint8_t length, uint8_t pos);
void DisplayNumber(u16 number);
void DisplayNumberMasked(u8 number, u8 mask = 0b1111);
void Clear();

#define AK_TM1637_IMPLEMENTATION

#ifdef AK_TM1637_IMPLEMENTATION


#define AK_TM1637_SPEED_LOW 1
#define AK_TM1637_SPEED_HIGH 2
#define AK_TM1637_SPEED_ULTRA 3

#ifndef AK_TM1637_SPEED
#define AK_TM1637_SPEED AK_TM1637_SPEED_HIGH
#endif
//#define AK_TM1637_SPEED AK_TM1637_ULTRA_SPEED

#if (AK_TM1637_SPEED == AK_TM1637_SPEED_LOW)
// 1kHz
#define AK_TM1637_CLOCK_PERIOD_US 1000 
#define AK_TM1637_CLOCK_HALF_PERIOD_US 500
#define AK_TM1637_CLOCK_QUARTER_PERIOD_US 250
#elif (AK_TM1637_SPEED == AK_TM1637_SPEED_HIGH)
// 6.25kHz
#define AK_TM1637_CLOCK_PERIOD_US 160
#define AK_TM1637_CLOCK_HALF_PERIOD_US 80
#define AK_TM1637_CLOCK_QUARTER_PERIOD_US 40
#elif (AK_TM1637_SPEED == AK_TM1637_SPEED_ULTRA)
// 6.25kHz
// for some reason, can't work at higher speed
#define AK_TM1637_CLOCK_PERIOD_US 160 
#define AK_TM1637_CLOCK_HALF_PERIOD_US 40
#define AK_TM1637_CLOCK_QUARTER_PERIOD_US 40
#endif

#define AK_TM1637_CMD_SETUP           0b01000000
#define AK_TM1637_CMD_SET_DATA        0b11000000

#define AK_TM1637_CMD_CONTROL           0b10000000
#define AK_TM1637_CMD_DISPLAY_ON_MASK   0b00001000
#define AK_TM1637_CMD_BRIGHTNESS_MASK   0b00000111

#define AK_TM1637_MASK_POS             0b11

inline void SetDioOutput()
{
    SetOutput(DISPLAY_DIO);
}
inline void SetDioInput()
{
    SetInput(DISPLAY_DIO);
}
inline uint8_t ReadDio()
{
    return ReadInput(DISPLAY_DIO);
}

inline void SetClkOutput()
{
    SetOutput(DISPLAY_CLK);
}

inline void SetClkInput()
{
    SetInput(DISPLAY_CLK);
}

#define AK_WAIT()    _delay_us(AK_TM1637_CLOCK_HALF_PERIOD_US)
#define TICK_HALF() _delay_us(AK_TM1637_CLOCK_HALF_PERIOD_US)
#define TICK_4() _delay_us(AK_TM1637_CLOCK_QUARTER_PERIOD_US)
#define CLK_HIGH() SetClkInput()
#define CLK_LOW() SetClkOutput()
#define DIO_HIGH() SetDioInput()
#define DIO_LOW() SetDioOutput()

inline void StartComm()
{
    DIO_LOW();
    TICK_4();
}

inline void EndComm()
{
    CLK_LOW(); // Ack is released
    DIO_LOW();
    TICK_HALF();

    CLK_HIGH(); // END
    TICK_4();
    DIO_HIGH();
    TICK_4();
}

inline uint8_t WriteByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        CLK_LOW();
        TICK_4();

        // Set data bit
        if (data & 1)
        {
            DIO_HIGH();
        }
        else
        {
            DIO_LOW();
        }
        TICK_4();

        CLK_HIGH();
        TICK_HALF();

        data >>= 1;
    }

    // Prepare for receiving acknowledgement
    CLK_LOW();
    DIO_HIGH();
    TICK_HALF();

    // Read acknowledgement
    CLK_HIGH();
    TICK_4();
    uint8_t ack = ReadDio();
    if (ack == 0) // Not strictly necessary, but it's a good idea
    {
        DIO_LOW(); // to avoid unnecessary voltage changes on DIO when possible
    }
    //CLK_LOW();
    TICK_4();

    return ack;
}

void Init()
{
    // set pins as hi-z (input, pullup disabled)
    // display board will pull the lines up
    SetInput(DISPLAY_CLK);
    SetInput(DISPLAY_DIO);
    SetLow(DISPLAY_CLK);
    SetLow(DISPLAY_DIO);
    TICK_HALF();
}

void SetBrightness(u8 brightness)
{
    if (brightness == 0)
    {
        StartComm();
        WriteByte(AK_TM1637_CMD_CONTROL);
        EndComm();
    }
    else
    {
        brightness = brightness - 1;
        if (brightness > 8) brightness = 8;
        StartComm();
        WriteByte(AK_TM1637_CMD_CONTROL | AK_TM1637_CMD_DISPLAY_ON_MASK | (AK_TM1637_CMD_BRIGHTNESS_MASK & brightness));
        EndComm();
    }
}

#define AK_TM1637_BLANK 0x10

static const uint8_t digitToSegment[] =
    {
        // XGFEDCBA
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111, // 9
        0b01110111, // A
        0b01111100, // b
        0b00111001, // C
        0b01011110, // d
        0b01111001, // E
        0b01110001, // F
        0b00000000, // BLANK
};

void SetSegments(const uint8_t segments[], uint8_t length, uint8_t pos)
{
    StartComm();
    // Write TM1637_CMD_SET_DATA + first digit address
    WriteByte(AK_TM1637_CMD_SET_DATA | (pos & AK_TM1637_MASK_POS));
    // Write data bytes
    for (uint8_t i = 0; i < length; i++)
    {
        WriteByte(digitToSegment[segments[i]]);
    }
    EndComm();
}

void DisplayNumber(u16 number)
{
    u8 segments[4];
    segments[3] = number % 10;
    segments[2] = (number / 10) % 10;
    segments[1] = (number / 100) % 10;
    segments[0] = (number / 1000) % 10;
    TM1637::SetSegments(segments, 4, 0);
}

void DisplayNumberMasked(u8 number, u8 mask)
{
    u8 segments;
    if (mask & 0b1000)
    {
        segments = (number / 1000) % 10;
        TM1637::SetSegments(&segments, 1, 0);
    }
    if (mask & 0b100)
    {
        segments = (number / 100) % 10;
        TM1637::SetSegments(&segments, 1, 1);
    }
    if (mask & 0b10)
    {
        segments = (number / 10) % 10;
        TM1637::SetSegments(&segments, 1, 2);
    }
    if (mask & 0b1)
    {
        segments = number % 10;
        TM1637::SetSegments(&segments, 1, 3);
    }
}

void Clear()
{
    uint8_t values[] = {
        AK_TM1637_BLANK, AK_TM1637_BLANK, AK_TM1637_BLANK, AK_TM1637_BLANK
    };
    
    SetSegments(values, 4, 0);
}

#endif // AK_TM1637_IMPLEMENTATION

}; // namespace TM1637

#endif // AK_TM1637_H