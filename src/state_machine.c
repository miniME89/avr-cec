#include "includes/state_machine.h"
#include "includes/defines.h"
#include "includes/cec.h"

State currentState = START;
bool eventTriggeredTimerA = false;
bool eventTriggeredTimerB = false;
bool eventToggledEdge = false;
bool eventStateTransistionIn = true;
bool eventStateTransistionOut = false;

ReadStartBitState readStartBitState = NOT_FOUND;
bool timerTaskAReset = false;
bool timerTaskBReset = false;
bool timerTaskARepeat = false;
bool timerTaskBRepeat = false;

uint8_t dataByteCounter = 0;
uint8_t dataBitCounter = 0;
char dataByte = 0;
char data[20];

uint16_t lastEdgeTicks = 0;
Level lastEdgeLevel = HIGH;

/************************************************************************/
/* state machine                                                        */
/************************************************************************/
void stateMachine()
{
	switch(currentState)
	{
		//============================================================================================================
		case START:
			if (isEventTransistionIn())
			{
				debug("a");
			}

			data[0] = 0x05;
			data[1] = 0x44;
			data[2] = 0x43;
			data[3] = '\0';
			setState(WRITE_SIGNAL_FREE_TIME);

			//setState(READ_START_BIT);

			if (isEventTransistionOut())
			{
				debug("a");
			}
		break;
		//============================================================================================================
		case EXIT:
			if (isEventTransistionIn())
			{
				debug("b");
			}

			if (isEventTransistionOut())
			{
				debug("b");
			}
		break;
		//============================================================================================================
		case READ_START_BIT:
			if (isEventTransistionIn())
			{
				debug("c");
			}

			if (isEventInputToggled())
			{
				if (lastEdgeLevel == HIGH)									//high
				{
					if (readStartBitState == READ_T0)						//is t1?
					{
						if ((START_BIT_T1 + START_BIT_TOLERANCE) > lastEdgeTicks && lastEdgeTicks > (START_BIT_T1 - START_BIT_TOLERANCE))	//t1 valid time?
						{
							readStartBitState = READ_T1;
						}
						else												//wrong timing
						{
							readStartBitState = NOT_FOUND;
						}
					}
				}
				else														//low
				{
					if (readStartBitState == NOT_FOUND)						//is t0?
					{
						readStartBitState = READ_T0;
						resetTimer1();
					}
					else if (readStartBitState == READ_T1)					//is t2?
					{
						if ((START_BIT_T2 + START_BIT_TOLERANCE) > lastEdgeTicks && lastEdgeTicks > (START_BIT_T2 - START_BIT_TOLERANCE))	//t2 valid time?
						{
							readStartBitState = READ_T2;
							//uartSendChar('F');

							setState(READ_DATA_BIT);
						}
						else												//timeout t2?
						{
							readStartBitState = NOT_FOUND;
						}
					}
				}
			}

			if (isEventTransistionOut())
			{
				debug("c");
			}
		break;
		//============================================================================================================
		case READ_DATA_BIT:
			if (isEventTransistionIn())
			{
				debug("d");
				dataBitCounter = 0;
				dataByteCounter = 0;
				dataByte = 0;
				eventToggledEdge = true;
			}

			if (isEventInputToggled())
			{
				if (lastEdgeLevel == LOW)							//start of next data bit
				{
					resetTimer1();

					setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_A, false, true);
					if (dataByteCounter == 0 && dataBitCounter == 9 && (dataByte & 0x0F) == LOGICAL_ADDRESS)			//is ACK bit and needs to be asserted?
					{
						cecLow();
						setTimeout(DATA_BIT_LOGIC_0, TIMER_B, false, false);
					}
					else
					{
						setTimeout(DATA_BIT_FOLLOWING_TIMEOUT, TIMER_B, true, false);
					}
				}
			}

			if (isEventTriggeredTimerA())
			{
				//debug("A");

				if (dataBitCounter <= 7)							//data bit
				{
					dataByte = (dataByte << 1) | lastEdgeLevel;
				}
				else if (dataBitCounter == 8)						//EOM
				{

				}
				else if (dataBitCounter == 9)						//ACK
				{

				}

				dataBitCounter++;

				if (dataBitCounter > 9)
				{
					uartSendChar(dataByte);
					data[dataByteCounter] = dataByte;
					dataBitCounter = 0;
					dataByteCounter++;
				}
			}

			if (isEventTriggeredTimerB())
			{
				if (dataByteCounter > 0 && dataBitCounter == 0 && (dataByte & 0x0F) == LOGICAL_ADDRESS)
				{
					cecHigh();
					setTimeout(DATA_BIT_FOLLOWING_TIMEOUT, TIMER_B, true, false);
				}
				else
				{
					//debug("B");

					clearTimeout(TIMER_A);
					clearTimeout(TIMER_B);

					readStartBitState = NOT_FOUND;
					setState(READ_START_BIT);

					if (dataBitCounter != 0)
					{
						//this shouldn't happen => error
						uartSendChar('X');
					}

					uartSendChar('\n');
					data[++dataByteCounter] = '\n';
				}
			}

			if (isEventTransistionOut())
			{
				debug("d");
			}
		break;
		//============================================================================================================
		case WRITE_SIGNAL_FREE_TIME:
			if (isEventTransistionIn())
			{
				debug("e");
			}

			//TODO: check when cec bus is free...
			cecLow();

			resetTimer1();

			setState(WRITE_START_BIT);
			setTimeout(START_BIT_T1, TIMER_A, false, false);
			setTimeout(START_BIT_T2, TIMER_B, true, false);

			if (isEventTransistionOut())
			{
				debug("e");
			}
		break;
		//============================================================================================================
		case WRITE_START_BIT:
			if (isEventTransistionIn())
			{
				debug("f");
			}

			if (isEventTriggeredTimerA())
			{
				cecHigh();
			}

			if (isEventTriggeredTimerB())
			{
				cecLow();
				setState(WRITE_DATA_BIT);
				setTimeout(DATA_BIT_FOLLOWING, TIMER_B, true, true);
				eventTriggeredTimerB = true;
			}

			if (isEventTransistionOut())
			{
				debug("f");
			}
		break;
		//============================================================================================================
		case WRITE_DATA_BIT:
			if (isEventTransistionIn())
			{
				debug("g");
			}

			if (isEventTriggeredTimerA())
			{
				cecHigh();
			}

			if (isEventTriggeredTimerB())
			{
				if (dataBitCounter <= 7)												//data bit
				{
					cecLow();

					bool dataBit = (data[dataByteCounter] & (0b10000000 >> dataBitCounter)) > 0;
					if (dataBit)														//high
					{
						setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
					}
					else																//low
					{
						setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
					}

					dataBitCounter++;
				}
				else if (dataBitCounter == 8)											//EOM
				{
					cecLow();

					if (data[dataByteCounter + 1])										//there is a following data block (EOM = 0)
					{
						setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
					}
					else																//there is no following data block (EOM = 1)
					{
						setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
					}

					dataBitCounter++;
				}
				else if (dataBitCounter == 9)											//ACK
				{
					cecLow();

					setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);

					dataBitCounter++;

					if (data[dataByteCounter + 1])										//there is a following data block
					{
						dataBitCounter = 0;
						dataByteCounter++;
					}
				}
				else
				{
					clearTimeout(TIMER_B);												//repeating task b will be disabled, therefore no following bits
					setState(READ_START_BIT);
				}
			}

			if (isEventTransistionOut())
			{
				debug("g");
			}
		break;
	}
}

bool isEvent(void)
{
	return (eventTriggeredTimerA || eventTriggeredTimerB || eventToggledEdge || eventStateTransistionIn || eventStateTransistionOut);
}

bool isEventTriggeredTimerA()
{
	if (eventTriggeredTimerA)
	{
		eventTriggeredTimerA = false;
		return true;
	}

	return false;
}

bool isEventTriggeredTimerB()
{
	if (eventTriggeredTimerB)
	{
		eventTriggeredTimerB = false;
		return true;
	}

	return false;
}

bool isEventInputToggled()
{
	if (eventToggledEdge)
	{
		eventToggledEdge = false;
		return true;
	}

	return false;
}

bool isEventTransistionIn()
{
	if (eventStateTransistionIn)
	{
		eventStateTransistionIn = false;
		return true;
	}

	return false;
}

bool isEventTransistionOut()
{
	if (eventStateTransistionOut)
	{
		eventStateTransistionOut = false;
		return true;
	}

	return false;
}

/************************************************************************/
/* set timeout                                                          */
/************************************************************************/
void setTimeout(uint16_t ticks, Timer timer, bool reset, bool repeat)
{
	if (timer == TIMER_A)
	{
		setTimer1CompareMatch(TIMER_A, ticks);
		setTimer1CompareMatchInterrupt(TIMER_A, true);
		timerTaskAReset = reset;
		timerTaskARepeat = repeat;
		eventTriggeredTimerA = false;
	}
	else
	{
		setTimer1CompareMatch(TIMER_B, ticks);
		setTimer1CompareMatchInterrupt(TIMER_B, true);
		timerTaskBReset = reset;
		timerTaskBRepeat = repeat;
		eventTriggeredTimerB = false;
	}
}

/************************************************************************/
/* clear timeout                                                        */
/************************************************************************/
void clearTimeout(Timer timer)
{
	setTimer1CompareMatchInterrupt(timer, false);
}

/************************************************************************/
/* set state                                                            */
/************************************************************************/
void setState(State state)
{
	currentState = state;
	eventStateTransistionIn = true;
	eventStateTransistionOut = true;
}


/************************************************************************/
/* Interrupt for timer1 input compare                                   */
/************************************************************************/
void stateMachineTimer1InputCapture()
{
	lastEdgeLevel = getInputCaptureState();
	lastEdgeTicks = getTimer1Ticks();
	eventToggledEdge = true;
	setInfoLED(lastEdgeLevel);

	if (lastEdgeLevel == HIGH)
	{
		setInputCaptureTriggerEdge(FALLING);						//next trigger on falling edge
	}
	else
	{
		setInputCaptureTriggerEdge(RISING);							//next trigger on rising edge
	}
}

/************************************************************************/
/* Interrupt for timer1 compare match A                                 */
/************************************************************************/
void stateMachineTimer1ACompareMatch()
{
	if (timerTaskAReset)
	{
		resetTimer1();
	}

	if (!timerTaskARepeat)
	{
		setTimer1CompareMatchInterrupt(TIMER_A, false);			//disable timer1 compare interrupt
	}

	eventTriggeredTimerA = true;
}

/************************************************************************/
/* Interrupt for timer1 compare match B                                 */
/************************************************************************/
void stateMachineTimer1BCompareMatch()
{
	if (timerTaskBReset)
	{
		resetTimer1();
	}

	if (!timerTaskBRepeat)
	{
		setTimer1CompareMatchInterrupt(TIMER_B, false);			//disable timer1 compare interrupt
	}

	eventTriggeredTimerB = true;
}

/************************************************************************/
/* Interrupt for UART received                                          */
/************************************************************************/
void stateMachineUartReceived(char c)
{
	uartSendChar('R');
}
