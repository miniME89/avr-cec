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
 * @file state_machine.c
 * @author Marcel
 * @brief
 *
 */

#include "includes/state_machine.h"
#include "includes/defines.h"
#include "includes/cec.h"
#include <avr/io.h>

State currentState = START;
SubStateReadStartBit readStartBitState = NOT_FOUND;
Events events = {.triggeredTimerA = false, .triggeredTimerB = false, .toggledEdge = false, .transistionIn = true, .transistionOut = false};

TimerOptions timerOptionsA = {.repeat = false, .reset = false};
TimerOptions timerOptionsB = {.repeat = false, .reset = false};

uint8_t dataByteCounter = 0;
uint8_t dataBitCounter = 0;
char data[16];
Level EOM;
Level ACK;

uint16_t lastEdgeTicks = 0;
Level lastEdgeLevel = HIGH;

bool send = false;

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

            if (send)
            {
                setState(WRITE_SIGNAL_FREE_TIME);
                uartSendString(data);
                uartSendChar('\n');
                send = false;
            }

            if (isEventInputToggled())
            {
                if (lastEdgeLevel == HIGH)
                {
                    if (readStartBitState == READ_T0)                       //is t1?
                    {
                        if ((START_BIT_T1 + START_BIT_TOLERANCE) > lastEdgeTicks && lastEdgeTicks > (START_BIT_T1 - START_BIT_TOLERANCE))   //t1 valid time?
                        {
                            readStartBitState = READ_T1;
                        }
                        else                                                //wrong timing
                        {
                            readStartBitState = NOT_FOUND;
                        }
                    }
                }
                else
                {
                    if (readStartBitState == NOT_FOUND)                     //is t0?
                    {
                        readStartBitState = READ_T0;
                        resetTimer1();
                    }
                    else if (readStartBitState == READ_T1)                  //is t2?
                    {
                        if ((START_BIT_T2 + START_BIT_TOLERANCE) > lastEdgeTicks && lastEdgeTicks > (START_BIT_T2 - START_BIT_TOLERANCE))   //t2 valid time?
                        {
                            readStartBitState = READ_T2;
                            //uartSendChar('F');

                            setState(READ_DATA_BIT);
                        }
                        else                                                //timeout t2?
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
                EOM = LOW;
                ACK = HIGH;

                events.toggledEdge = true;
            }

            if (isEventInputToggled())
            {
                if (lastEdgeLevel == LOW)                                   //start of next data bit
                {
                    resetTimer1();
                    setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_A, false, false);

                    if (dataBitCounter == 9 && (data[0] & 0x0F) == LOGICAL_ADDRESS)     //is ACK bit and needs to be asserted?
                    {
                        cecLow();                                           //assert ACK bit start
                        setTimeout(DATA_BIT_LOGIC_0, TIMER_B, false, false);
                    }
                }
                else if (EOM == HIGH)                                       //end of message
                {
                    clearTimeout(TIMER_A);
                    clearTimeout(TIMER_B);

                    readStartBitState = NOT_FOUND;
                    setState(READ_START_BIT);

                    uartSendChar('\n');
                }
            }

            if (isEventTriggeredTimerA())
            {
                if (dataBitCounter <= 7)                                    //data bit
                {
                    data[dataByteCounter] = (data[dataByteCounter] << 1) | lastEdgeLevel;
                }

                if (dataBitCounter == 7)                                    //last information bit of data block
                {
                    uartSendChar(data[dataByteCounter]);
                }
                else if (dataBitCounter == 8)                               //EOM bit
                {
                    EOM = lastEdgeLevel;
                }
                else if (dataBitCounter == 9)                               //ACK bit
                {
                    ACK = lastEdgeLevel;
                }

                dataBitCounter++;

                if (dataBitCounter > 9)                                     //end of data block (10 bit)
                {
                    dataBitCounter = 0;
                    dataByteCounter++;

                    if (dataByteCounter > 15)                               //max CEC message size is 16 blocks
                    {
                        //TODO error handling
                        setState(READ_START_BIT);
                    }
                }
            }

            if (isEventTriggeredTimerB())
            {
                uartSendChar('R');
                cecHigh();                                                  //assert ACK bit end
            }

            if (isEventTransistionOut())
            {
                debug("d");

                dataBitCounter = 0;
                dataByteCounter = 0;
            }
        break;
        //============================================================================================================
        case WRITE_SIGNAL_FREE_TIME:
            if (isEventTransistionIn())
            {
                debug("e");

                //TODO check for correct signal free time: SFT_PRESENT_INITIATOR, SFT_NEW_INITIATOR or SFT_RETRANSMISSION
                //NOTE: According to the specification the SFT is the time since the last transmitted frame. Due to limitations of the timer unit
                //      the signal free time is checked from the beginning of a writing attempt. This will delay the signal at max 16.8ms.
                resetTimer1();
                setTimeout(SFT_NEW_INITIATOR, TIMER_A, true, false);
            }

            if (isEventInputToggled())
            {
                resetTimer1();
            }

            if (isEventTriggeredTimerA())
            {
                setState(WRITE_START_BIT);
            }

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

                cecLow();
                resetTimer1();

                setTimeout(START_BIT_T1, TIMER_A, false, false);
                setTimeout(START_BIT_T2, TIMER_B, true, false);
            }

            if (isEventInputToggled())
            {
                //TODO implement proper monitoring..
                if (cecShouldLevel() != lastEdgeLevel)                  //monitor CEC line
                {
                    //TODO error handling: unexpected HIGH/LOW impedance on line
                    cecHigh();

                    clearTimeout(TIMER_A);
                    clearTimeout(TIMER_B);

                    setState(READ_START_BIT);

                    uartSendChar('L');
                }
            }

            if (isEventTriggeredTimerA())
            {
                cecHigh();
            }

            if (isEventTriggeredTimerB())
            {
                setState(WRITE_DATA_BIT);
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

                dataBitCounter = 0;
                dataByteCounter = 0;

                events.triggeredTimerA = true;
            }

            if (isEventTriggeredTimerA())
            {
                if (lastEdgeLevel == HIGH)                                                  //start of next data bit
                {
                    if (dataBitCounter >= 10)                                               //data block finished?
                    {
                        if (data[dataByteCounter + 1])                                      //there is a following data block
                        {
                            dataBitCounter = 0;
                            dataByteCounter++;
                        }
                        else                                                                //end of message
                        {
                            setState(READ_START_BIT);
                        }
                    }

                    if (dataBitCounter <= 7)                                                //information bit
                    {
                        cecLow();
                        setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);            //sample information bit (for verification)

                        bool dataBit = (data[dataByteCounter] & (0b10000000 >> dataBitCounter)) > 0;
                        if (dataBit)                                                        //logic 1
                        {
                            setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                        }
                        else                                                                //logic 0
                        {
                            setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
                        }

                        dataBitCounter++;
                    }
                    else if (dataBitCounter == 8)                                           //EOM
                    {
                        cecLow();
                        setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);            //sample EOM bit (for verification)

                        if (data[dataByteCounter + 1])                                      //there is a following data block (EOM = 0)
                        {
                            setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
                        }
                        else                                                                //there is no following data block (EOM = 1)
                        {
                            setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                        }

                        dataBitCounter++;
                    }
                    else if (dataBitCounter == 9)                                           //ACK
                    {
                        cecLow();

                        setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                        setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);            //sample ACK bit (for verification)

                        dataBitCounter++;
                    }
                }
                else                                                                        //back to high impedance of data bit
                {
                    cecHigh();
                    setTimeout(DATA_BIT_FOLLOWING, TIMER_A, true, false);                   //set timer for following data bit
                }
            }

            if (isEventTriggeredTimerB())
            {
                if (dataBitCounter < 10)                                                    //verify bit on CEC line
                {
                    if (cecShouldLevel() != lastEdgeLevel)                                  //monitor CEC line
                    {
                        //TODO error handling: unexpected HIGH/LOW impedance on line
                        cecHigh();

                        clearTimeout(TIMER_A);
                        clearTimeout(TIMER_B);

                        setState(READ_START_BIT);

                        uartSendChar('L');
                    }
                }
                else                                                                        //don't verify ACK bit
                {
                    if (lastEdgeLevel == HIGH)                                              //ACK bit not asserted by follower?
                    {
                        //TODO error handling: ACK bit not asserted by follower
                        uartSendChar('E');
                    }
                }
            }

            if (isEventTransistionOut())
            {
                debug("g");

                dataBitCounter = 0;
                dataByteCounter = 0;
            }
        break;
    }
}

bool isEvent(void)
{
    return (events.triggeredTimerA || events.triggeredTimerB || events.toggledEdge || events.transistionIn || events.transistionOut);
}

bool isEventTriggeredTimerA()
{
    if (events.triggeredTimerA)
    {
        events.triggeredTimerA = false;
        return true;
    }

    return false;
}

bool isEventTriggeredTimerB()
{
    if (events.triggeredTimerB)
    {
        events.triggeredTimerB = false;
        return true;
    }

    return false;
}

bool isEventInputToggled()
{
    if (events.toggledEdge)
    {
        events.toggledEdge = false;
        return true;
    }

    return false;
}

bool isEventTransistionIn()
{
    if (events.transistionIn)
    {
        events.transistionIn = false;
        return true;
    }

    return false;
}

bool isEventTransistionOut()
{
    if (events.transistionOut)
    {
        events.transistionOut = false;
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
        timerOptionsA.reset = reset;
        timerOptionsA.repeat = repeat;
        events.triggeredTimerA = false;
    }
    else
    {
        setTimer1CompareMatch(TIMER_B, ticks);
        setTimer1CompareMatchInterrupt(TIMER_B, true);
        timerOptionsB.reset = reset;
        timerOptionsB.repeat = repeat;
        events.triggeredTimerB = false;
    }
}

/************************************************************************/
/* clear timeout                                                        */
/************************************************************************/
void clearTimeout(Timer timer)
{
    setTimer1CompareMatchInterrupt(timer, false);
    if (timer == TIMER_A)
    {
        events.triggeredTimerA = false;
    }
    else
    {
        events.triggeredTimerB = false;
    }
}

/************************************************************************/
/* set state                                                            */
/************************************************************************/
void setState(State state)
{
    currentState = state;
    events.transistionIn = true;
    events.transistionOut = true;
}


/************************************************************************/
/* Interrupt for timer1 input compare                                   */
/************************************************************************/
void stateMachineTimer1InputCapture()
{
    lastEdgeLevel = getInputCaptureState();
    lastEdgeTicks = getTimer1Ticks();
    events.toggledEdge = true;
    setInfoLED(lastEdgeLevel);

    if (lastEdgeLevel == HIGH)
    {
        setInputCaptureTriggerEdge(FALLING);                        //next trigger on falling edge
    }
    else
    {
        setInputCaptureTriggerEdge(RISING);                         //next trigger on rising edge
    }
}

/************************************************************************/
/* Interrupt for timer1 compare match A                                 */
/************************************************************************/
void stateMachineTimer1ACompareMatch()
{
    if (timerOptionsA.reset)
    {
        resetTimer1();
    }

    if (!timerOptionsA.repeat)
    {
        setTimer1CompareMatchInterrupt(TIMER_A, false);         //disable timer1 compare A interrupt
    }

    events.triggeredTimerA = true;
}

/************************************************************************/
/* Interrupt for timer1 compare match B                                 */
/************************************************************************/
void stateMachineTimer1BCompareMatch()
{
    if (timerOptionsB.reset)
    {
        resetTimer1();
    }

    if (!timerOptionsB.repeat)
    {
        setTimer1CompareMatchInterrupt(TIMER_B, false);         //disable timer1 compare B interrupt
    }

    events.triggeredTimerB = true;
}

/************************************************************************/
/* Interrupt for UART received                                          */
/************************************************************************/
void stateMachineUartReceived(char c)
{
    //TODO temporary + need a proper end of message character
    if (c == '\n')
    {
        data[dataByteCounter++] = '\0';
        send = true;
    }
    else
    {
        data[dataByteCounter++] = c;
    }
}
