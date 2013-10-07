#include "includes/peripherals.h"
#include "includes/state_machine.h"
#include "includes/defines.h"
#include "includes/utils.h"
#include "includes/cec.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

FIFOBuffer* bufferUart;
uint8_t timer1OverflowCounter = 0;

/************************************************************************/
/* interrupts                                                           */
/************************************************************************/
void setInterrupts(bool enable)
{
	if (enable)
	{
		sei();
	}
	else
	{
		cli();
	}
}

/************************************************************************/
/* Interrupt for timer1 input compare                                   */
/************************************************************************/

//Interrupt for timer1 input compare
ISR(TIMER1_CAPT_vect)
{
	stateMachineTimer1InputCapture();
}

//Interrupt for timer1 compare match A
ISR(TIMER1_COMPA_vect)
{
	stateMachineTimer1ACompareMatch();
}

//Interrupt for timer1 compare match B
ISR(TIMER1_COMPB_vect)
{
	stateMachineTimer1BCompareMatch();
}

//Interrupt for timer1 overflow
ISR(TIMER1_OVF_vect)
{
	timer1OverflowCounter++;
}

/************************************************************************/
/* IO                                                                   */
/************************************************************************/
void initIO(void)
{
	INFO_LED_PORT_DDR |= (1 << INFO_LED_PIN);
	setDirectionCEC(INPUT);
}

Level getInputCaptureState()
{
	return (CEC_INPUT_CAPTURE_PORT & (1 << CEC_INPUT_CAPTURE_PIN));
}

void setInputCaptureTriggerEdge(Edge edge)
{
	if (edge == FALLING)
	{
		TCCR1B &= ~(1 << ICES1);									//trigger on falling edge
	}
	else
	{
		TCCR1B |= (1 << ICES1);										//trigger on rising edge
	}
}

void setInfoLED(Level level)
{
	if (((1 << INFO_LED_PIN) & (level << INFO_LED_PIN)) > 0)
	{
		INFO_LED_PORT |= (1 << INFO_LED_PIN);
	}
	else
	{
		INFO_LED_PORT &= ~(1 << INFO_LED_PIN);
	}
	
}

void setDirectionCEC(DataDirection direction)
{
	if (direction == OUTPUT)
	{
		CEC_BUS_PULL_PORT &= ~(1 << CEC_BUS_PULL_PIN);				//LOW
		CEC_BUS_PULL_PORT_DDR |= (1 << CEC_BUS_PULL_PIN);			//output
	}
	else
	{
		CEC_BUS_PULL_PORT &= ~(1 << CEC_BUS_PULL_PIN);				//disable pull up
		CEC_BUS_PULL_PORT_DDR &= ~(1 << CEC_BUS_PULL_PIN);			//input
	}
}

/************************************************************************/
/* timer1                                                               */
/************************************************************************/
void initTimer1()
{
	//TCCR1B = (1 << WGM12);

	#if TIMER1_PRESCALER == 1
		TCCR1B |= (0 << ICES1) | (1 << CS10);						//input capture (falling edge) + 1x prescaler
	#elif TIMER1_PRESCALER == 8
		TCCR1B |= (0 << ICES1) | (1 << CS11);						//input capture (falling edge) + 8x prescaler
	#elif TIMER1_PRESCALER == 64
		TCCR1B |= (0 << ICES1) | (1 << CS11) | (1 << CS10);			//input capture (falling edge) + 64x prescaler
	#elif TIMER1_PRESCALER == 256
		TCCR1B |= (0 << ICES1) | (1 << CS12);						//input capture (falling edge) + 256x prescaler
	#elif TIMER1_PRESCALER == 1024
		TCCR1B |= (0 << ICES1) | (1 << CS12) | (1 << CS10);			//input capture (falling edge) + 1024x prescaler
	#else
		#warning "value of TIMER1_PRESCALER needs to be 1, 8, 64, 256 or 1024"
	#endif

	TIMSK |= (1 << TICIE1) | (1 << TOIE1);							//interrupt on input capture + interrupt on timer overflow
}

void resetTimer1()
{
	TCNT1 = 0;
	timer1OverflowCounter = 0;
}

uint16_t getTimer1Ticks(void)
{
	return TCNT1;
}

void setTimer1CompareMatch(Timer timer, uint16_t ticks)
{
	if (timer == TIMER_A)
	{
		OCR1A = ticks;
	}
	else
	{
		OCR1B = ticks;
	}
}

void setTimer1CompareMatchInterrupt(Timer timer, bool enable)
{
	if (enable)
	{
		if (timer == TIMER_A)
		{
			TIMSK |= (1 << OCIE1A);
		}
		else
		{
			TIMSK |= (1 << OCIE1B);
		}
	}
	else
	{
		if (timer == TIMER_A)
		{
			TIMSK &= ~(1 << OCIE1A);
		}
		else
		{
			TIMSK &= ~(1 << OCIE1B);
		}
	}
}

/************************************************************************/
/* uart                                                                 */
/************************************************************************/
void initUart(void)
{
	unsigned int regubrr = UBRR_REGISTER;							//set baud rate
	UBRRH = (unsigned char)(regubrr >> 8);
	UBRRL = (unsigned char)regubrr;

	UCSRB = (1 << RXEN) | (1 << TXEN);								//enable receiver and transmitter

	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);				//set frame format: 8data

	bufferUart = newBufferFIFO(64);
}

void uart_write_char(unsigned char data)
{
	while(!( UCSRA & (1 << UDRE)))
	{
		if (eventToggledEdge || eventTriggeredTimerA || eventTriggeredTimerB)
		{
			return;
		}
	}

	UDR = data;
}

void uart_write_string(char* str)
{
	while (*str)
	{
		uart_write_char(*str);
		str++;
	}
}
