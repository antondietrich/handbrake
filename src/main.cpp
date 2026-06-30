#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#if DEBUG
#define DISPLAY_CLK F7
#define DISPLAY_DIO F6
#define AK_TM1637_SPEED AK_TM1637_SPEED_HIGH
#define AK_TM1637_IMPLEMENTATION
#include "ak_tm1637s.h"
#endif

#define FORCE_CALIBRATION_SUCCESS 1
#define FORCE_SEND_REPORT 1

#include "avr_common.h"
#include "avr_debug.h"
#include "avr_eeprom.h"

u32 led1time = 0;
u32 led2time = 0;
void Led1Off();
void Led2Off();
void Led1On();
void Led2On();
void Led1Blink();
void Led2Blink();

#define TC0_INTERRUPTA_ENABLE() 	TIMSK0 = _BV(OCIE0A)
#define TC0_INTERRUPTA_DISABLE() 	TIMSK0 = 0
void TickAsync();

#include "handbrake.h"
#include "descriptors.cpp"
#include "usb_endpoint.cpp"
#include "usb_device.cpp"

enum class State : u8
{
    STARTUP,
    CALIBRATION,
    USB
};

void SetupHardware();
void EnterState(State newState);

static State handbrakeState = State::STARTUP;

static u32 millis = 0;
static u32 seconds = 0;
static u8 hall = 0;
static u8 hallMin = 0;
static u8 hallMax = 0;
static float hallRemapScale = 1;
static u32 calibrationStartTime = 0;
static u8 lastHall = 0;
static u32 lastReportTime = 0;
static u8 hallMinDeadzone = 1;

void TickAsync()
{
	++millis;
	if (millis % 1000 == 0)
	{
		++seconds;
	}
}

ISR(TIMER0_COMPA_vect)
{
    TickAsync();
}

int main(void)
{
	SetupHardware();

#if DEBUG
    TM1637::Init();
    TM1637::Clear();
    TM1637::SetBrightness(1);
#endif

    Led1Off();
    Led2Off();
    DEBUG_OUT(DEBUG_STAGE_START);

    u8 calibartionDone = EepromReadByte(E2ADDR_CALIBRATION_DONE);
    if (calibartionDone != 1)
    {
        EnterState(State::CALIBRATION);
    }
    else
    {
        EnterState(State::USB);
    }

	sei();

    u8 hallCount = 0;
    u16 hallStartSum = 0;
    u32 lastUpdateTime = 0;
    u8 sleepMS = 4;
    u16 calibrationBlinkTime = 0;

    set_sleep_mode(SLEEP_MODE_IDLE);

	for (;;)
	{
        /* Process Sleep */
        cli();
        if (millis - lastUpdateTime < sleepMS)
        {
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
            continue;
        }
        sei();

        lastUpdateTime = millis;

        /* Update LEDs */
        if (led1time != 0 && millis >= led1time)
        {
            Led1Off();
        }

        if (led2time != 0 && millis >= led2time)
        {
            Led2Off();
        }

        /* Read Hall Sensor*/
        ADCSRA |= _BV(ADSC); // start conversion, wait till it's 0 to complete
        while (ADCSRA & _BV(ADSC));
        hall = ADCH;

        /* Main Update */
        if (handbrakeState == State::CALIBRATION)
        {
            if (millis - calibrationBlinkTime >= 1000)
            {
                calibrationBlinkTime = millis;
                Led2Blink();
            }

            if (millis - calibrationStartTime > 5000)
            {
                if (AbsDistance(hallMin, hallMax) > 64 || FORCE_CALIBRATION_SUCCESS)
                {
                    EepromWriteByte(1, E2ADDR_CALIBRATION_DONE);
                    EepromWriteByte(hallMin, E2ADDR_HALL_MIN);
                    EepromWriteByte(hallMax, E2ADDR_HALL_MAX);
                    Led1Blink();
                    EnterState(State::USB);
                    continue;
                }
                else
                {
                    Led1On();
                    Led2On();
                    DEBUG_OUT(DEBUG_ERROR_CALIBRATION, DEBUG_PRIORITY_ERROR, 1);
                    cli();
                    return 1;
                }
            }

            if (hallCount < 10)
            {
                hallStartSum += hall;
                ++hallCount;
            }
            else if (hallCount == 10)
            {
                hallMin = hallStartSum / 10;
                hallMax = hallMin;
                ++hallCount;
            }
            else
            {
                if (AbsDistance(hall, hallMin) > AbsDistance(hallMax, hallMin))
                {
                    hallMax = hall;
                }
            }
        }
        else if (handbrakeState == State::USB)
        {
            USBDeviceUpdate();
            if (gUSBState == USBDeviceState::CONFIGURED && !EP1_IsHalted())
            {
                u8 reportChanged = 0;

                if (hallMax > hallMin)
                {
                    hall = Max(hall, hallMin);
                    hall = Min(hall, hallMax);
                }
                else
                {
                    hall = Min(hall, hallMin);
                    hall = Max(hall, hallMax);
                }

                if (AbsDistance(hall, hallMin) < hallMinDeadzone && hall != hallMin)
                {
                    hall = hallMin;
                    reportChanged = 1;
                }
                else
                {
                    reportChanged = AbsDistance(hall, lastHall) > 1;
                }

                if (reportChanged || (gHIDIdleTime > 0 && (millis - lastReportTime) > gHIDIdleTime) || FORCE_SEND_REPORT)
                {
                    EP_SELECT(1); 
                    if (EP_IS_IN_BANK_READY())
                    {
                        u32 elapsed = millis - lastReportTime;
                        //DEBUG_OUT((u16)(elapsed), DEBUG_PRIORITY_MIN + 2);
                        lastReportTime = millis;

                        lastHall = hall;
                        u8 brake = (u8)(AbsDistance(hall, hallMin) * hallRemapScale);
                        Report report;
                        report.id = 1;
                        report.brake = brake;
                        EP_SendBuffer((u8*)&report, sizeof(Report));
                    }
                }
            }
        }
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
    USB_INTERRUPTS_CLEAR();
    USB_INTERRUPTS_OFF();

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
    TC0_INTERRUPTA_ENABLE();

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


void Led1Off()
{
	PORTD |= (1 << PD5);
    led1time = 0;
}

void Led2Off()
{
	PORTB |= (1 << PB0);
    led2time = 0;
}

void Led1On() 
{
	PORTD &= ~(1 << PD5);
    led1time = 0;
}

void Led2On()
{
	PORTB &= ~(1 << PB0);
    led2time = 0;
}

void Led1Blink() 
{
	PORTD &= ~(1 << PD5);
    led1time = millis + 50;
}

void Led2Blink()
{
	PORTB &= ~(1 << PB0);
    led2time = millis + 50;
}

void EnterState(State newState)
{
    if (handbrakeState == newState)
    {
        return;
    }

    handbrakeState = newState;

    if (handbrakeState == State::CALIBRATION)
    {
        calibrationStartTime = millis;
    }
    else if (handbrakeState == State::USB)
    {
        hallMin = EepromReadByte(E2ADDR_HALL_MIN);
        hallMax = EepromReadByte(E2ADDR_HALL_MAX);
        hallRemapScale = 255.0f / AbsDistance(hallMin, hallMax);
        USBInit();
    }
}