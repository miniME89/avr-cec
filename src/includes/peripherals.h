#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

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

extern FIFOBuffer* bufferUart;
extern uint8_t timer1OverflowCounter;

//interrupts
void setInterrupts(bool enable);

//IO
void initIO(void);
Level getInputCaptureState(void);
void setInputCaptureTriggerEdge(Edge edge);
void setInfoLED(Level level);
void setDirectionCEC(DataDirection direction);

//Timer1
void initTimer1(void);
void resetTimer1(void);
uint16_t getTimer1Ticks(void);
void setTimer1CompareMatch(Timer timer, uint16_t ticks);
void setTimer1CompareMatchInterrupt(Timer timer, bool enable);

//UART
void initUart(void);
void uart_write_char(unsigned char data);
void uart_write_string(char* str);

#endif
