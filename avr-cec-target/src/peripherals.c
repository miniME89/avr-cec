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
#include "queue.h"
#include "cec.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//==========================================
// Variables
//==========================================
static InterruptCallback callbackInputCapture;
static InterruptCallback callbackTimerA;
static InterruptCallback callbackTimerB;

static uint8_t timerOverflowCounter = 0;

//==========================================
// Definitions
//==========================================
void peripheralsSetup()
{
    //setup IO
    __CONCAT(DDR, INFO_LED_PORT) |= (1 << INFO_LED_PIN);
    setOutCECLevel(HIGH);
    setOutInfoLEDLevel(HIGH);

    //setup timer
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

Level getInCECLevel()
{
    return (__CONCAT(PIN, CEC_INPUT_PORT) & (1 << CEC_INPUT_PIN));
}

void setOutCECLevel(Level level)
{
    if (level == LOW)
    {
        /*
         * setting the CEC output pin to output and pulling the pin low will fore the CEC bus to be LOW, therefore a LOW bit is written on the bus
         */
        __CONCAT(PORT, CEC_OUTPUT_PORT) &= ~(1 << CEC_OUTPUT_PIN);      //LOW
        __CONCAT(DDR, CEC_OUTPUT_PORT) |= (1 << CEC_OUTPUT_PIN);           //output
    }
    else
    {
        /*
         * setting the CEC output pin to input and disabling the pull will cause the CEC output pin to be disconnected from the CEC bus. Because the
         * default state of the bus is HIGH (which is pulled up by the other connected devices) a HIGH bit is written on the bus (as long as no other
         * device pulled the bus LOW).
         */
        __CONCAT(PORT, CEC_OUTPUT_PORT) &= ~(1 << CEC_OUTPUT_PIN);         //disable pull up
        __CONCAT(DDR, CEC_OUTPUT_PORT) &= ~(1 << CEC_OUTPUT_PIN);          //input
    }
}

void setOutInfoLEDLevel(Level level)
{
    if (((1 << INFO_LED_PIN) & (level << INFO_LED_PIN)) > 0)
    {
        __CONCAT(PORT, INFO_LED_PORT) |= (1 << INFO_LED_PIN);
    }
    else
    {
        __CONCAT(PORT, INFO_LED_PORT) &= ~(1 << INFO_LED_PIN);
    }

}

void registerCallbackInputCapture(InterruptCallback callback)
{
    callbackInputCapture = callback;
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

void setValueTimerCompareMatch(Timer timer, uint16_t ticks)
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

void setInterruptTimerCompareMatch(Timer timer, bool enable)
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

void registerCallbackTimerA(InterruptCallback callback)
{
    callbackTimerA = callback;
}

void registerCallbackTimerB(InterruptCallback callback)
{
    callbackTimerB = callback;
}

//Interrupt for timer input compare
ISR(TIMER1_CAPT_vect)
{
    callbackInputCapture();

    TCCR1B ^= (1 << ICES1);
}

//Interrupt for timer compare match A
ISR(TIMER1_COMPA_vect)
{
    callbackTimerA();
}

//Interrupt for timer compare match B
ISR(TIMER1_COMPB_vect)
{
    callbackTimerB();
}

//Interrupt for timer overflow
ISR(TIMER1_OVF_vect)
{
    timerOverflowCounter++;
}
