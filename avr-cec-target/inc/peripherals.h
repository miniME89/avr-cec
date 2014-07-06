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
 * @file peripherals.h
 * @author Marcel
 * @brief
 *
 */

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_

#include <inttypes.h>
#include <stdbool.h>

typedef void (*InterruptCallback)(void);

typedef enum Level
{
    LOW = 0,
    HIGH = 1
} Level;

typedef enum Timer
{
    TIMER_A,
    TIMER_B
} Timer;

/**
 * Setup peripherals.
 */
void peripheralsSetup(void);

/**
 * Enable or disable global interrupts
 * @param enable True to enable global interrupts or false to disable global interrupts.
 */
void setInterrupts(bool enable);

//==========================================
// IO
//==========================================
/**
 * Get the level of the current level on the CEC input pin. The CEC input pin is used to only read the CEC bus.
 * @return Returns the current level of the CEC input pin. This can be HIGH or LOW.
 */
Level getInCECLevel(void);

/**
 * Set the level of the CEC output pin. The CEC output pin is used to only write the CEC bus. This will apply the level to the whole CEC bus. Note that a
 * LOW level will force the bus LOW until it is set HIGH again. In the meantime no other devices can use the bus. The HIGH level will simply disconnect the
 * CEC output pin from the bus which will result in the default HIGH state of the bus.
 * @param level The level to set on the CEC output pin. This can be HIGH or LOW.
 */
void setOutCECLevel(Level level);

/**
 * Set the level of the output info LED pin.
 * @param level The level to set on the output info LED pin. This can be HIGH or LOW.
 */
void setOutInfoLEDLevel(Level level);

/**
 *
 * @param callback
 */
void registerCallbackInputCapture(InterruptCallback callback);

//==========================================
// Timer
//==========================================
/**
 *
 */
void resetTimer(void);

/**
 *
 * @return
 */
uint16_t getTimerTicks(void);

/**
 *
 * @return
 */
uint8_t getTimerOverflowCounter(void);

/**
 *
 * @param timer
 * @param ticks
 */
void setValueTimerCompareMatch(Timer timer, uint16_t ticks);

/**
 *
 * @param timer
 * @param enable
 */
void setInterruptTimerCompareMatch(Timer timer, bool enable);

/**
 *
 * @param callback
 */
void registerCallbackTimerA(InterruptCallback callback);

/**
 *
 * @param callback
 */
void registerCallbackTimerB(InterruptCallback callback);

#endif
