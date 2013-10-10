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

#include "utils.h"
#include <inttypes.h>

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

extern uint8_t timer1OverflowCounter;

//interrupts
void setInterrupts(bool enable);

//IO
void initIO(void);
Level getInputCaptureState(void);
void setInputCaptureTriggerEdge(Edge edge);
void setInfoLED(Level level);
void setDataDirectionCEC(DataDirection direction);

//Timer1
void initTimer1(void);
void resetTimer1(void);
uint16_t getTimer1Ticks(void);
void setTimer1CompareMatch(Timer timer, uint16_t ticks);
void setTimer1CompareMatchInterrupt(Timer timer, bool enable);

//UART
void initUart(void);
void uartSendChar(char c);
void uartSendString(char* str);
void uartFlush(void);
bool uartReadChar(char* c);
bool uartReadString(char* str, uint8_t size);

#endif
