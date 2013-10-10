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
 * @file state_machine.h
 * @author Marcel
 * @brief
 *
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include "peripherals.h"
#include <inttypes.h>
#include <stdbool.h>

typedef enum State {
    START,
    EXIT,
    READ_START_BIT,
    READ_DATA_BIT,
    WRITE_SIGNAL_FREE_TIME,
    WRITE_START_BIT,
    WRITE_DATA_BIT
} State;

typedef enum SubStateReadStartBit {
    NOT_FOUND,                  //no start bit found
    READ_T0,                    //read t0 edge => any negative edge
    READ_T1,                    //read t1 edge in specified time
    READ_T2                     //read t2 edge in specified time => found start bit
} SubStateReadStartBit;

typedef struct Events
{
    bool triggeredTimerA;
    bool triggeredTimerB;
    bool toggledEdge;
    bool transistionIn;
    bool transistionOut;
} Events;

typedef struct TimerOptions
{
    bool repeat;                //repeat
    bool reset;                 //reset timer1 on execution
} TimerOptions;

void stateMachine(void);
void setState(State state);

bool isEvent(void);
bool isEventTriggeredTimerA(void);
bool isEventTriggeredTimerB(void);
bool isEventInputToggled(void);
bool isEventTransistionIn(void);
bool isEventTransistionOut(void);

void setTimeout(uint16_t ticks, Timer timer, bool reset, bool repeat);
void clearTimeout(Timer timer);

void stateMachineTimer1InputCapture(void);
void stateMachineTimer1ACompareMatch(void);
void stateMachineTimer1BCompareMatch(void);
void stateMachineUartReceived(char c);

#endif
