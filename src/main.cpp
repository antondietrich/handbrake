#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include "avr_common.h"
#include "avr_debug.h"
#include "main.h"

#if DEBUG
u32 led1time = 0;
u32 led2time = 0;
void Led1Off();
void Led2Off();
void Led1On();
void Led2On();
#else
#define Led1Off()
#define Led2Off()
#define Led1On()
#define Led2On()
#endif



#include "descriptors.cpp"
#include "usb_endpoint.cpp"
#include "usb_device.cpp"
#include "handbrake.h"

static u8 numSetupRequests = 0;

u32 millis = 0;
u32 seconds = 0;

u8 hall = 0;


ISR(TIMER0_COMPA_vect)
{
	++millis;
	if (millis % 1000 == 0)
	{
		++seconds;
        if (seconds % 2 == 0)
        {
            Led1On();
        }
	}
}

u8 usbReady = 0;
u32 ts = 0;
int main(void)
{
	SetupHardware();
	USBInit();

    Led1Off();
    Led2Off();
    DEBUG_OUT(DEBUG_STAGE_START);

    // TODO: lufa uses memory barriers here
	sei();

	for (;;)
	{
#if DEBUG && 1
        if (millis == led1time)
        {
            led1time = 0;
            Led1Off();
        }
        if (millis == led2time)
        {
            led2time = 0;
            Led2Off();
        }
#endif

		USB_DeviceTask();
        if (gUSBState == USBDeviceState::CONFIGURED)
        {
            EP_SELECT(1);
            if (EP_IS_IN_BANK_READY())
            {
                Led2On();
                Report report;
                report.brake = 0;
                //EP_SendBuffer((u8*)&report, sizeof(Report));
                EP_SendBuffer((u8*)&report, 1);
            }
        }

		//ts = millis + 10;
		//while (usbReady && millis < ts) USB_USBTask();
        #if 1
		ADCSRA |= _BV(ADSC); // start conversion, wait till it's 0 to complete
		while (ADCSRA & _BV(ADSC));
		hall = ADCH;
        #endif
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Set up LEDs */
	DDRD |= _BV(PD5); // led 1
    PORTD |= _BV(PD5);
	DDRB |= _BV(PB0); // led 2
    PORTB |= _BV(PB0);

	/* Set up TC0 for ms timing */
	OCR0A = 249; // output compare value
	TCCR0A |= _BV(WGM01); // CTC mode - count up to OCR0A and reset
	TCCR0B |= _BV(CS01) | _BV(CS00); // 1/64 prescaler
	TCNT0 = 0;
	TIMSK0 |= _BV(OCIE0A); // compare match A interrupt enabled

    /* Set T/C1 output A as PWM output on PB5 (Digital 9) */
#if DEBUG
    DDRB |= _BV(PB5);
    TCCR1A = _BV(COM1A1) | _BV(WGM10); // phase-correct non-inverting 8bit PWM
    TCCR1B =  _BV(CS10);
    TCNT1 = 0;
    OCR1A = 3;
#endif

	/* Set up ADC */
	// ADMUX - voltage ref, gain
	// enable ADC - set ADEN in ADSCRA
	// TODO: disable adc (clear ADEN) before going into power saving
	// result in ADCH, ADCL (left-adjust and read ADCH for 8 bit precision)
	// start conversion - write 1 to ADSC. the bit is cleare by HW when completed (there's also an interrupt)
	// verify that AREF is connected
	// TODO: noise reduction mode
	DDRD &= ~_BV(PD4);
	PORTD &= ~_BV(PD4);
	ADMUX = _BV(ADLAR) | _BV(REFS0); // left-adjust, use AVCC reference
	ADCSRB = _BV(MUX5); // select ADC8 channel (D4 pin)
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // enable ADC
	//ADCSRA |= _BV(ADSC); // start conversion, wait till it's 0 to complete
	DIDR2 |= _BV(ADC8D); // disable digital input on ADC8 pin
}


#if DEBUG
void Led1Off()
{
	PORTD |= (1 << PD5);
}

void Led2Off()
{
	PORTB |= (1 << PB0);
}

void Led1On() 
{
	PORTD &= ~(1 << PD5);
    led1time = millis + 10;
}

void Led2On()
{
	PORTB &= ~(1 << PB0);
    led2time = millis + 10;
}
#endif
