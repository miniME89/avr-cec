/*
 *  Copyright (C) 2013
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file peripherals.c
 * @author Marcel
 * @brief
 *
 */

#include "peripherals.h"
#include "defines.h"
#include "utils.h"
#include "cec.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//==========================================
// Variables
//==========================================
static CharQueue* queueUartWrite;
static CharQueue* queueUartRead;
static uint8_t timerOverflowCounter = 0;

//==========================================
// Definitions
//==========================================
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

//IO
void initIO(void)
{
    INFO_LED_PORT_DDR |= (1 << INFO_LED_PIN);
    setDataDirectionCEC(INPUT);
    setInfoLED(HIGH);
}

Level getInputCaptureState()
{
    return (CEC_INPUT_CAPTURE_PORT & (1 << CEC_INPUT_CAPTURE_PIN));
}

void setInputCaptureTriggerEdge(Edge edge)
{
    if (edge == FALLING)
    {
        TCCR1B &= ~(1 << ICES1);                            //trigger on falling edge
    }
    else
    {
        TCCR1B |= (1 << ICES1);                             //trigger on rising edge
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

void setDataDirectionCEC(DataDirection direction)
{
    if (direction == OUTPUT)
    {
        CEC_BUS_PULL_PORT &= ~(1 << CEC_BUS_PULL_PIN);      //LOW
        CEC_BUS_PULL_PORT_DDR |= (1 << CEC_BUS_PULL_PIN);   //output
    }
    else
    {
        CEC_BUS_PULL_PORT &= ~(1 << CEC_BUS_PULL_PIN);      //disable pull up
        CEC_BUS_PULL_PORT_DDR &= ~(1 << CEC_BUS_PULL_PIN);  //input
    }
}

//timer
void initTimer()
{
    //TCCR1B = (1 << WGM12);

    #if TIMER_PRESCALER == 1
        TCCR1B |= (0 << ICES1) | (1 << CS10);               //input capture (falling edge) + 1x prescaler
    #elif TIMER_PRESCALER == 8
        TCCR1B |= (0 << ICES1) | (1 << CS11);               //input capture (falling edge) + 8x prescaler
    #elif TIMER_PRESCALER == 64
        TCCR1B |= (0 << ICES1) | (1 << CS11) | (1 << CS10); //input capture (falling edge) + 64x prescaler
    #elif TIMER_PRESCALER == 256
        TCCR1B |= (0 << ICES1) | (1 << CS12);               //input capture (falling edge) + 256x prescaler
    #elif TIMER_PRESCALER == 1024
        TCCR1B |= (0 << ICES1) | (1 << CS12) | (1 << CS10); //input capture (falling edge) + 1024x prescaler
    #else
        #warning "value of TIMER_PRESCALER needs to be 1, 8, 64, 256 or 1024"
    #endif

    TIMSK |= (1 << TICIE1) | (1 << TOIE1);                  //interrupt on input capture + interrupt on timer overflow
}

void resetTimer()
{
    TCNT1 = 0;
    timerOverflowCounter = 0;
}

uint16_t getTimerTicks(void)
{
    return TCNT1 ;
}

uint8_t getTimerOverflowCounter()
{
    return timerOverflowCounter;
}

void setTimerCompareMatch(Timer timer, uint16_t ticks)
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

void setTimerCompareMatchInterrupt(Timer timer, bool enable)
{
    if (enable)
    {
        if (timer == TIMER_A)
        {
            TIFR |= (1 << OCF1A);
            TIMSK |= (1 << OCIE1A);
        }
        else
        {
            TIFR |= (1 << OCF1B);
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

//Interrupt for timer input compare
ISR(TIMER1_CAPT_vect)
{
    executeTimerInputCapture();
}

//Interrupt for timer compare match A
ISR(TIMER1_COMPA_vect)
{
    executeTimerACompareMatch();
}

//Interrupt for timer compare match B
ISR(TIMER1_COMPB_vect)
{
    executeTimerBCompareMatch();
}

//Interrupt for timer overflow
ISR(TIMER1_OVF_vect)
{
    timerOverflowCounter++;
}

//uart
void initUart(void)
{
    unsigned int regubrr = UBRR_REGISTER;                   //set baud rate
    UBRRH = (unsigned char) (regubrr >> 8);
    UBRRL = (unsigned char) regubrr;

    UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);       //enable receiver + transmitter + receiver interrupt

    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);     //set frame format: 8data

    queueUartWrite = newQueueChar(2);
    queueUartRead = newQueueChar(2);
}

void uartSendChar(char c)
{
    putChar(queueUartWrite, c);
}

void uartSendString(char* str)
{
    while (*str)
    {
        putChar(queueUartWrite, *str);
        str++;
    }
}

void uartFlush()
{
    char c;
    uint8_t charCount = 0;

    while (charCount < FLUSH_MAX_CHARS && !isEmptyQueueChar(queueUartWrite))
    {
        while (!(UCSRA & (1 << UDRE)))                      //wait till UART is ready
        {
            if (isEvent())                                  //cancel if there is a state machine event
            {
                return;
            }
        }

        getChar(queueUartWrite, &c);

        UDR = c;
        charCount++;
    }
}

bool uartReadChar(char* c)
{
    return getChar(queueUartRead, c);
}

bool uartReadString(char* str, uint8_t size)
{
    uint8_t count = 0;
    while (getChar(queueUartRead, str) && count < size)
    {
        str++;
    }

    if (count > 0)
    {
        str++;
        *str = '\0';

        return true;
    }
    else
    {
        return false;
    }
}

ISR(USART_RXC_vect)
{
    putChar(queueUartRead, UDR);
}
