#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <inttypes.h>
#include <stdbool.h>
#include "peripherals.h"

typedef enum ReadStartBitState {
	NOT_FOUND,					//no start bit found
	READ_T0,					//read t0 edge => any negative edge
	READ_T1,					//read t1 edge in specified time
	READ_T2						//read t2 edge in specified time => found start bit
} ReadStartBitState;

typedef enum State {
	START,
	EXIT,
	READ_START_BIT,
	READ_DATA_BIT,
	WRITE_SIGNAL_FREE_TIME,
	WRITE_START_BIT,
	WRITE_DATA_BIT
} State;

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
