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

typedef enum Level
{
    LOW = 0,
    HIGH = 1
} Level;

typedef enum Edge
{
    FALLING,
    RISING
} Edge;

typedef enum Timer
{
    TIMER_A,
    TIMER_B
} Timer;

typedef enum DataDirection
{
    INPUT,
    OUTPUT
} DataDirection;

//interrupts
void setInterrupts(bool enable);

//IO
void initIO(void);
Level getInputCaptureState(void);
void setInputCaptureTriggerEdge(Edge edge);
void setInfoLED(Level level);
void setDataDirectionCEC(DataDirection direction);

//Timer1
void initTimer(void);
void resetTimer(void);
uint16_t getTimerTicks(void);
uint8_t getTimerOverflowCounter(void);
void setTimerCompareMatch(Timer timer, uint16_t ticks);
void setTimerCompareMatchInterrupt(Timer timer, bool enable);

#endif