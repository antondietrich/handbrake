#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include "avr_common.h"

/* Interrupts should be disabled by external code during EEPROM access! */

void EepromWriteByte(u8 value, u16 address)
{
    while(EECR & _BV(EEPE));
    EEAR = address;
    EEDR = value;
    EECR |= _BV(EEMPE); // will be cleared by hw in 4 cycles
    EECR |= _BV(EEPE); // write strobe
}

u8 EepromReadByte(u16 address)
{
    while(EECR & _BV(EEPE));
    EEAR = address;
    EECR |= _BV(EERE); // read strobe
    u8 result = EEDR;
    return result;
}