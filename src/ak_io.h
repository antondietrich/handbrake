#ifndef AK_IO_H
#define AK_IO_H

/*
 * Helper functions for register manipulation and basic I/O
 * 
 * Basic functions:
 * 
 * SetBit(REGISTER, BIT);
 * ClearBit(REGISTER, BIT);
 * 
 * Example:
 * 
 * SetBit(TCCR0B, CS02); // set bit 2 of TCCR0B register to 1
 * SetBit(TCCR0B, 2); // same
 * ClearBit(TCCR0B, CS02);
 * ClearBit(TCCR0B, 2);
 * 
 * Higher-level functions for I/O registers:
 * 
 * I/O pins are aliased by their port letter and bit, e.g. B3 refers to bit 3 of port B.
 * Arduino aliases can also be used, e.g. D13 for digital pin 13 (aka B5).
 * 
 * SetInput(PIN_ALIAS);
 * SetOutput(PIN_ALIAS);
 * SetLow(PIN_ALIAS);
 * SetHigh(PIN_ALIAS);
 * 
 * Example:
 * 
 * SetInput(PB0);        // Set pin 0 of DDRB register to 0
 * SetOutput(D13);      // Set pin 5 of DDRB register to 1
 * SetLow(PB4);          // Set pin 4 of PORTB register to 0
 * SetHigh(PB4);         // Set pin 4 of PORTB register to 1
 */

#ifdef __AVR_ATmega328P__

#define PORT_B0_ PORTB
#define DDR_B0_ DDRB
#define PIN_B0_ PINB
#define BIT_B0_ 0
#define PORT_B1_ PORTB
#define DDR_B1_ DDRB
#define PIN_B1_ PINB
#define BIT_B1_ 1
#define PORT_B2_ PORTB
#define DDR_B2_ DDRB
#define PIN_B2_ PINB
#define BIT_B2_ 2
#define PORT_B3_ PORTB
#define DDR_B3_ DDRB
#define PIN_B3_ PINB
#define BIT_B3_ 3
#define PORT_B4_ PORTB
#define DDR_B4_ DDRB
#define PIN_B4_ PINB
#define BIT_B4_ 4
#define PORT_B5_ PORTB
#define DDR_B5_ DDRB
#define PIN_B5_ PINB
#define BIT_B5_ 5
#define PORT_B6_ PORTB
#define DDR_B6_ DDRB
#define PIN_B6_ PINB
#define BIT_B6_ 6
#define PORT_B7_ PORTB
#define DDR_B7_ DDRB
#define PIN_B7_ PINB
#define BIT_B7_ 7

#define PORT_C0_ PORTC
#define DDR_C0_ DDRC
#define PIN_C0_ PINC
#define BIT_C0_ 0
#define PORT_C1_ PORTC
#define DDR_C1_ DDRC
#define PIN_C1_ PINC
#define BIT_C1_ 1
#define PORT_C2_ PORTC
#define DDR_C2_ DDRC
#define PIN_C2_ PINC
#define BIT_C2_ 2
#define PORT_C3_ PORTC
#define DDR_C3_ DDRC
#define PIN_C3_ PINC
#define BIT_C3_ 3
#define PORT_C4_ PORTC
#define DDR_C4_ DDRC
#define PIN_C4_ PINC
#define BIT_C4_ 4
#define PORT_C5_ PORTC
#define DDR_C5_ DDRC
#define PIN_C5_ PINC
#define BIT_C5_ 5
#define PORT_C6_ PORTC
#define DDR_C6_ DDRC
#define PIN_C6_ PINC
#define BIT_C6_ 6

#define PORT_D0_ PORTD
#define DDR_D0_ DDRD
#define PIN_D0_ PIND
#define BIT_D0_ 0
#define PORT_D1_ PORTD
#define DDR_D1_ DDRD
#define PIN_D1_ PIND
#define BIT_D1_ 1
#define PORT_D2_ PORTD
#define DDR_D2_ DDRD
#define PIN_D2_ PIND
#define BIT_D2_ 2
#define PORT_D3_ PORTD
#define DDR_D3_ DDRD
#define PIN_D3_ PIND
#define BIT_D3_ 3
#define PORT_D4_ PORTD
#define DDR_D4_ DDRD
#define PIN_D4_ PIND
#define BIT_D4_ 4
#define PORT_D5_ PORTD
#define DDR_D5_ DDRD
#define PIN_D5_ PIND
#define BIT_D5_ 5
#define PORT_D6_ PORTD
#define DDR_D6_ DDRD
#define PIN_D6_ PIND
#define BIT_D6_ 6
#define PORT_D7_ PORTD
#define DDR_D7_ DDRD
#define PIN_D7_ PIND
#define BIT_D7_ 7

/* ATmega Mapping */

#define B1 B1_
#define B2 B2_
#define B3 B3_
#define B4 B4_
#define B5 B5_
#define B6 B6_
#define B7 B7_

#define C0 C0_
#define C1 C1_
#define C2 C2_
#define C3 C3_
#define C4 C4_
#define C5 C5_
#define C6 C6_

#define D0 D0_
#define D1 D1_
#define D2 D2_
#define D3 D3_
#define D4 D4_
#define D5 D5_
#define D6 D6_
#define D7 D7_

/* Arduino Mapping */
#ifdef ARDUINO_NANO

#define DIGITAL0      D0_
#define DIGITAL1      D1_
#define DIGITAL2      D2_
#define DIGITAL3      D3_
#define DIGITAL4      D4_
#define DIGITAL5      D5_
#define DIGITAL6      D6_
#define DIGITAL7      D7_

#define DIGITAL8      B0_
#define DIGITAL9      B1_
#define DIGITAL10     B2_
#define DIGITAL11     B3_
#define DIGITAL12     B4_
#define DIGITAL13     B5_

#define DIGITAL14     C0_
#define DIGITAL15     C1_
#define DIGITAL16     C2_
#define DIGITAL17     C3_
#define DIGITAL18     C4_
#define DIGITAL19     C5_

#define ANALOG0      C0_
#define ANALOG1      C1_
#define ANALOG2      C2_
#define ANALOG3      C3_
#define ANALOG4      C4_
#define ANALOG5      C5_

#endif // ARDUINO_NANO

#endif // __AVR_ATmega328P__

#ifdef __AVR_ATmega32U4__

#define PORT_B0_    PORTB
#define DDR_B0_     DDRB
#define PIN_B0_     PINB
#define BIT_B0_     0
#define PORT_B1_    PORTB
#define DDR_B1_     DDRB
#define PIN_B1_     PINB
#define BIT_B1_     1
#define PORT_B2_    PORTB
#define DDR_B2_     DDRB
#define PIN_B2_     PINB
#define BIT_B2_     2
#define PORT_B3_    PORTB
#define DDR_B3_     DDRB
#define PIN_B3_     PINB
#define BIT_B3_     3
#define PORT_B4_    PORTB
#define DDR_B4_     DDRB
#define PIN_B4_     PINB
#define BIT_B4_     4
#define PORT_B5_    PORTB
#define DDR_B5_     DDRB
#define PIN_B5_     PINB
#define BIT_B5_     5
#define PORT_B6_    PORTB
#define DDR_B6_     DDRB
#define PIN_B6_     PINB
#define BIT_B6_     6
#define PORT_B7_    PORTB
#define DDR_B7_     DDRB
#define PIN_B7_     PINB
#define BIT_B7_     7

#define B0 B0_
#define B1 B1_
#define B2 B2_
#define B3 B3_
#define B4 B4_
#define B5 B5_
#define B6 B6_
#define B7 B7_

#define PORT_C0_    PORTC
#define DDR_C0_     DDRC
#define PIN_C0_     PINC
#define BIT_C0_     0
#define PORT_C1_    PORTC
#define DDR_C1_     DDRC
#define PIN_C1_     PINC
#define BIT_C1_     1
#define PORT_C2_    PORTC
#define DDR_C2_     DDRC
#define PIN_C2_     PINC
#define BIT_C2_     2
#define PORT_C3_    PORTC
#define DDR_C3_     DDRC
#define PIN_C3_     PINC
#define BIT_C3_     3
#define PORT_C4_    PORTC
#define DDR_C4_     DDRC
#define PIN_C4_     PINC
#define BIT_C4_     4
#define PORT_C5_    PORTC
#define DDR_C5_     DDRC
#define PIN_C5_     PINC
#define BIT_C5_     5
#define PORT_C6_    PORTC
#define DDR_C6_     DDRC
#define PIN_C6_     PINC
#define BIT_C6_     6
#define PORT_C7_    PORTC
#define DDR_C7_     DDRC
#define PIN_C7_     PINC
#define BIT_C7_     7

#define C0 C0_
#define C1 C1_
#define C2 C2_
#define C3 C3_
#define C4 C4_
#define C5 C5_
#define C6 C6_
#define C7 C7_

#define PORT_D0_    PORTD
#define DDR_D0_     DDRD
#define PIN_D0_     PIND
#define BIT_D0_     0
#define PORT_D1_    PORTD
#define DDR_D1_     DDRD
#define PIN_D1_     PIND
#define BIT_D1_     1
#define PORT_D2_    PORTD
#define DDR_D2_     DDRD
#define PIN_D2_     PIND
#define BIT_D2_     2
#define PORT_D3_    PORTD
#define DDR_D3_     DDRD
#define PIN_D3_     PIND
#define BIT_D3_     3
#define PORT_D4_    PORTD
#define DDR_D4_     DDRD
#define PIN_D4_     PIND
#define BIT_D4_     4
#define PORT_D5_    PORTD
#define DDR_D5_     DDRD
#define PIN_D5_     PIND
#define BIT_D5_     5
#define PORT_D6_    PORTD
#define DDR_D6_     DDRD
#define PIN_D6_     PIND
#define BIT_D6_     6
#define PORT_D7_    PORTD
#define DDR_D7_     DDRD
#define PIN_D7_     PIND
#define BIT_D7_     7

#define D0 D0_
#define D1 D1_
#define D2 D2_
#define D3 D3_
#define D4 D4_
#define D5 D5_
#define D6 D6_
#define D7 D7_

#define PORT_E0_    PORTE
#define DDR_E0_     DDRE
#define PIN_E0_     PINE
#define BIT_E0_     0
#define PORT_E1_    PORTE
#define DDR_E1_     DDRE
#define PIN_E1_     PINE
#define BIT_E1_     1
#define PORT_E2_    PORTE
#define DDR_E2_     DDRE
#define PIN_E2_     PINE
#define BIT_E2_     2
#define PORT_E3_    PORTE
#define DDR_E3_     DDRE
#define PIN_E3_     PINE
#define BIT_E3_     3
#define PORT_E4_    PORTE
#define DDR_E4_     DDRE
#define PIN_E4_     PINE
#define BIT_E4_     4
#define PORT_E5_    PORTE
#define DDR_E5_     DDRE
#define PIN_E5_     PINE
#define BIT_E5_     5
#define PORT_E6_    PORTE
#define DDR_E6_     DDRE
#define PIN_E6_     PINE
#define BIT_E6_     6
#define PORT_E7_    PORTE
#define DDR_E7_     DDRE
#define PIN_E7_     PINE
#define BIT_E7_     7

#define E0 E0_
#define E1 E1_
#define E2 E2_
#define E3 E3_
#define E4 E4_
#define E5 E5_
#define E6 E6_
#define E7 E7_

#define PORT_F0_    PORTF
#define DDR_F0_     DDRF
#define PIN_F0_     PINF
#define BIT_F0_     0
#define PORT_F1_    PORTF
#define DDR_F1_     DDRF
#define PIN_F1_     PINF
#define BIT_F1_     1
#define PORT_F2_    PORTF
#define DDR_F2_     DDRF
#define PIN_F2_     PINF
#define BIT_F2_     2
#define PORT_F3_    PORTF
#define DDR_F3_     DDRF
#define PIN_F3_     PINF
#define BIT_F3_     3
#define PORT_F4_    PORTF
#define DDR_F4_     DDRF
#define PIN_F4_     PINF
#define BIT_F4_     4
#define PORT_F5_    PORTF
#define DDR_F5_     DDRF
#define PIN_F5_     PINF
#define BIT_F5_     5
#define PORT_F6_    PORTF
#define DDR_F6_     DDRF
#define PIN_F6_     PINF
#define BIT_F6_     6
#define PORT_F7_    PORTF
#define DDR_F7_     DDRF
#define PIN_F7_     PINF
#define BIT_F7_     7

#define F0 F0_
#define F1 F1_
#define F2 F2_
#define F3 F3_
#define F4 F4_
#define F5 F5_
#define F6 F6_
#define F7 F7_

#endif

#define PORT_FOR_ALIAS(p) (PORT_##p)
#define PIN_FOR_ALIAS(p) (PIN_##p)
#define DDR_FOR_ALIAS(p) (DDR_##p)
#define BIT_FOR_ALIAS(p) (BIT_##p)

#define SetBit(p, b) (p) |= (1 << (b))
#define ClearBit(p, b) (p) &= ~(1 << (b))

#define SetInput_Impl(p) (ClearBit(DDR_FOR_ALIAS(p), BIT_FOR_ALIAS(p)))
#define SetOutput_Impl(p) (SetBit(DDR_FOR_ALIAS(p), BIT_FOR_ALIAS(p)))
#define SetLow_Impl(p) (ClearBit(PORT_FOR_ALIAS(p), BIT_FOR_ALIAS(p)))
#define SetHigh_Impl(p) (SetBit(PORT_FOR_ALIAS(p), BIT_FOR_ALIAS(p)))
#define ReadInput_Impl(p) ((PIN_FOR_ALIAS(p) >> BIT_FOR_ALIAS(p)) & 1)

/* DDR Low */
#define SetInput(p) (SetInput_Impl(p))
/* DDR Hight */
#define SetOutput(p) (SetOutput_Impl(p))
/* PORT Low */
#define SetLow(p) (SetLow_Impl(p))
/* PORT High */
#define SetHigh(p) (SetHigh_Impl(p))
/* read PIN bit */
#define ReadInput(p) (ReadInput_Impl(p))


#endif // AK_IO_H