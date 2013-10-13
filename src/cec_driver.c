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
 * @file cec_driver.c
 * @author Marcel
 * @brief
 *
 */

#include "includes/cec_driver.h"
#include "includes/defines.h"
#include "includes/peripherals.h"
#include "includes/utils.h"

//==========================================
// Declarations
//==========================================
/**
 * The state of the state machine.
 */
typedef enum State {
    START,                  //!< START
    EXIT,                   //!< EXIT
    READ_START_BIT,         //!< READ_START_BIT
    READ_DATA_BIT,          //!< READ_DATA_BIT
    WRITE_SIGNAL_FREE_TIME, //!< WRITE_SIGNAL_FREE_TIME
    WRITE_START_BIT,        //!< WRITE_START_BIT
    WRITE_DATA_BIT          //!< WRITE_DATA_BIT
} State;

/**
 * The sub states for the sub state machine when in READ_START_BIT state.
 */
typedef enum SubStateReadStartBit {
    NOT_FOUND,              //!< no start bit found
    READ_T0,                //!< read t0 edge => any negative edge
    READ_T1,                //!< read t1 edge in specified time
    READ_T2                 //!< read t2 edge in specified time => found start bit
} SubStateReadStartBit;

/**
 * All possible events of the state machine.
 */
typedef struct Events
{
    bool triggeredTimerA;
    bool triggeredTimerB;
    bool toggledEdge;
    bool transistionIn;
    bool transistionOut;
} Events;

/**
 * Timer options
 */
typedef struct TimerOptions
{
    bool repeat;            //!< repeat
    bool reset;             //!< reset timer1 on execution
} TimerOptions;

/**
 * Set the state for the state machine.
 * @param state
 */
static void setState(State state);

/**
 * Check if there is a triggered timer A event. This will consume the event if there is one.
 * @return Returns true if there is a triggered timer A event.
 */
static bool isEventTriggeredTimerA(void);

/**
 * Check if there is a triggered timer B event. This will consume the event if there is one.
 * @return Returns true if there is a triggered timer B event.
 */
static bool isEventTriggeredTimerB(void);

/**
 * Check if there is a input toggled event. This will consume the event if there is one.
 * @return Returns true if there is a input toggled event.
 */
static bool isEventInputToggled(void);

/**
 * Check if there is a state transition in event. This will consume the event if there is one.
 * @return Returns true if there is a state transition in event.
 */
static bool isEventTransistionIn(void);

/**
 * Check if there is a state transition out event. This will consume the event if there is one.
 * @return Returns true if there is a state transition out event.
 */
static bool isEventTransistionOut(void);

/**
 *
 * @param ticks
 * @param timer
 * @param reset
 * @param repeat
 */
static void setTimeout(uint16_t ticks, Timer timer, bool reset, bool repeat);

/**
 *
 * @param timer
 */
static void clearTimeout(Timer timer);

/**
 * Pull the CEC line low.
 * This will set the connected pin as output with logic 0 which will therefore pull the CEC line low.
 */
static void low(void);

/**
 * Set high impedance state for connected pin.
 * This will set the connected pin as input which is therefore in high impedance state and will no longer pull the CEC line low.
 */
static void high(void);

/**
 * Verify the level of the bus set by the functions low() and high().
 * Note: The change of the level by using low() or high() isn't immediate.
 * @return Returns true if the bus is on the expected level set by the low() and high() functions.
 */
static bool verifyLevel(void);

//==========================================
// Variables
//==========================================
static State currentState = START;
static SubStateReadStartBit readStartBitState = NOT_FOUND;
static Events events = {.triggeredTimerA = false, .triggeredTimerB = false, .toggledEdge = false, .transistionIn = true, .transistionOut = false};

static TimerOptions timerOptionsA = {.repeat = false, .reset = false};
static TimerOptions timerOptionsB = {.repeat = false, .reset = false};

static MessageQueue* messageQueueRead;
static MessageQueue* messageQueueWrite;
static Message messageBufferWrite;
static Message messageBufferRead;
static uint8_t bitCounter;
static uint8_t byteCounter;
static Level EOM;
static Level ACK;

static uint16_t lastEdgeTicks = 0;
static Level lastEdgeLevel = HIGH;
static Level shouldLevel = HIGH;

//==========================================
// Definitions
//==========================================
void initDriver(void)
{
    messageQueueRead = newQueueMessage(16);
    messageQueueWrite = newQueueMessage(16);
}

void processDriver()
{
    switch(currentState)
    {
        //============================================================================================================
        case START:
            if (isEventTransistionIn())
            {
                debug("a");
            }

            Message message;
            message.header = 0x05;
            message.opcode = 0x44;
            message.operands[0] = 0x43;
            message.size = 3;
            putMessage(messageQueueWrite, message);

            setState(READ_START_BIT);

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

            if (readStartBitState == NOT_FOUND)                             //registered nothing on line?
            {
                if (messageBufferWrite.size == 0)                           //no message in write buffer?
                {
                    if (getMessage(messageQueueWrite, &messageBufferWrite)) //get message from queue
                    {
                        setState(WRITE_SIGNAL_FREE_TIME);
                    }
                }
                else
                {
                    setState(WRITE_SIGNAL_FREE_TIME);
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

                messageBufferRead.size = 0;
                bitCounter = 0;
                byteCounter = 0;
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

                    if (bitCounter == 9 && (messageBufferRead.header & 0x0F) == LOGICAL_ADDRESS)     //is ACK bit and needs to be asserted?
                    {
                        #if ENABLE_ASSERTION == 1
                            low();                                          //assert ACK bit (start)
                            setTimeout(DATA_BIT_LOGIC_0, TIMER_B, false, false);
                        #endif
                    }
                }
                else if (EOM == HIGH)                                       //end of message
                {
                    clearTimeout(TIMER_A);
                    clearTimeout(TIMER_B);

                    messageBufferRead.size = byteCounter;
                    putMessage(messageQueueRead, messageBufferRead);

                    readStartBitState = NOT_FOUND;
                    setState(READ_START_BIT);
                }
            }

            //Timer A execution: on sample time (1.05ms) after start of every bit to read the logic level
            if (isEventTriggeredTimerA())
            {
                if (bitCounter <= 7)                                        //data bit
                {
                    if (byteCounter == 0)                                   //1 byte header
                    {
                        messageBufferRead.header = (messageBufferRead.header << 1) | lastEdgeLevel;
                    }
                    else if (byteCounter == 1)                              //0-1 byte opcode
                    {
                        messageBufferRead.opcode = (messageBufferRead.opcode << 1) | lastEdgeLevel;
                    }
                    else                                                    //0-14 bytes operands
                    {
                        messageBufferRead.operands[byteCounter - 2] = (messageBufferRead.operands[byteCounter - 2] << 1) | lastEdgeLevel;
                    }
                }

                if (bitCounter == 7)
                {
                    byteCounter++;

                    if (byteCounter > 15)                                   //max CEC message size is 16 blocks
                    {
                        //TODO error handling
                        setState(READ_START_BIT);
                    }
                }
                else if (bitCounter == 8)                                   //EOM bit
                {
                    EOM = lastEdgeLevel;
                }
                else if (bitCounter == 9)                                   //ACK bit
                {
                    ACK = lastEdgeLevel;
                }

                bitCounter++;

                if (bitCounter > 9)                                         //end of data block (10 bit)
                {
                    bitCounter = 0;
                }
            }

            //Timer B execution: pull line up again when data block was asserted
            if (isEventTriggeredTimerB())
            {
                #if ENABLE_ASSERTION == 1
                    uartSendChar('R');
                    high();                                                 //assert ACK bit (end)
                #endif
            }

            if (isEventTransistionOut())
            {
                debug("d");

                messageBufferRead.size = 0;
                bitCounter = 0;
                byteCounter = 0;
            }
        break;
        //============================================================================================================
        case WRITE_SIGNAL_FREE_TIME:
            if (isEventTransistionIn())
            {
                debug("e");

                //TODO check for correct signal free time: SFT_PRESENT_INITIATOR, SFT_NEW_INITIATOR or SFT_RETRANSMISSION
                resetTimer1();
                setTimeout(SFT_NEW_INITIATOR, TIMER_A, true, false);
            }

            if (isEventInputToggled())
            {
                clearTimeout(TIMER_A);
                setState(READ_START_BIT);
                events.toggledEdge = true;
                uartSendChar('S');
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

                low();
                resetTimer1();

                setTimeout(START_BIT_T1, TIMER_A, false, false);
                setTimeout(START_BIT_T2, TIMER_B, true, false);
            }

            if (isEventInputToggled())
            {
                //TODO implement proper monitoring..
                if (!verifyLevel())                                         //monitor CEC line
                {
                    //TODO error handling: unexpected HIGH/LOW impedance on line
                    high();

                    clearTimeout(TIMER_A);
                    clearTimeout(TIMER_B);

                    setState(READ_START_BIT);
                    events.toggledEdge = true;

                    uartSendChar('L');
                }
            }

            if (isEventTriggeredTimerA())
            {
                high();
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

                bitCounter = 0;
                byteCounter = 0;

                events.triggeredTimerA = true;
            }

            //Timer A execution: beginning of every bit to pull line LOW / when line needs to be pulled HIGH again (dependend of logic 1/0)
            if (isEventTriggeredTimerA())
            {
                if (lastEdgeLevel == HIGH)                                                  //start of next data bit
                {
                    if (bitCounter >= 10)                                                   //data block finished?
                    {
                        if ((byteCounter + 1) < messageBufferWrite.size)                     //there is a following data block
                        {
                            bitCounter = 0;
                            byteCounter++;
                        }
                        else                                                                //end of message
                        {
                            setState(READ_START_BIT);
                        }
                    }

                    if (bitCounter <= 7)                                                    //information bit
                    {
                        low();
                        setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);            //sample information bit (for verification)

                        bool dataBit;
                        if (byteCounter == 0)                                               //1 byte header
                        {
                            dataBit = (messageBufferWrite.header & (0b10000000 >> bitCounter)) > 0;
                        }
                        else if (byteCounter == 1)                                          //0-1 byte opcode
                        {
                            dataBit = (messageBufferWrite.opcode & (0b10000000 >> bitCounter)) > 0;
                        }
                        else                                                                //0-14 bytes operands
                        {
                            dataBit = (messageBufferWrite.operands[byteCounter - 2] & (0b10000000 >> bitCounter)) > 0;
                        }

                        if (dataBit)                                                        //logic 1
                        {
                            setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                        }
                        else                                                                //logic 0
                        {
                            setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
                        }

                        bitCounter++;
                    }
                    else if (bitCounter == 8)                                               //EOM
                    {
                        low();
                        setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);            //sample EOM bit (for verification)

                        if ((byteCounter + 1) < messageBufferWrite.size)                    //there is a following data block (EOM = 0)
                        {
                            setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
                        }
                        else                                                                //there is no following data block (EOM = 1)
                        {
                            setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                        }

                        bitCounter++;
                    }
                    else if (bitCounter == 9)                                               //ACK
                    {
                        low();

                        setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                        setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);            //sample ACK bit (for verification)

                        bitCounter++;
                    }
                }
                else                                                                        //back to high impedance of data bit
                {
                    high();
                    setTimeout(DATA_BIT_FOLLOWING, TIMER_A, true, false);                   //set timer for following data bit
                }
            }

            //Timer B execution: on sample time (1.05ms) after start of every bit to verify bit on line and check assertion of ACK bit
            if (isEventTriggeredTimerB())
            {
                if (bitCounter < 10)                                                        //verify bit on CEC line
                {
                    if (!verifyLevel())                                                     //monitor CEC line
                    {
                        //TODO error handling: unexpected HIGH/LOW impedance on line
                        high();

                        clearTimeout(TIMER_A);
                        clearTimeout(TIMER_B);

                        setState(READ_START_BIT);
                        events.toggledEdge = true;

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

                messageBufferWrite.size = 0;
                bitCounter = 0;
                byteCounter = 0;
            }
        break;
    }
}

void setState(State state)
{
    currentState = state;
    events.transistionIn = true;
    events.transistionOut = true;
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

bool writeMessage(Message message)
{
    return putMessage(messageQueueWrite, message);
}

bool readMessage(Message* message)
{
    return getMessage(messageQueueRead, message);
}

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

void low()
{
    setDataDirectionCEC(OUTPUT);
    shouldLevel = LOW;
}

void high()
{
    setDataDirectionCEC(INPUT);
    shouldLevel = HIGH;
}

bool verifyLevel()
{
    return (shouldLevel == lastEdgeLevel);
}

void executeTimer1InputCapture()
{
    lastEdgeLevel = getInputCaptureState();
    lastEdgeTicks = getTimer1Ticks();
    events.toggledEdge = true;
    setInfoLED(lastEdgeLevel);

    if (lastEdgeLevel == HIGH)
    {
        setInputCaptureTriggerEdge(FALLING);                    //next trigger on falling edge
    }
    else
    {
        setInputCaptureTriggerEdge(RISING);                     //next trigger on rising edge
    }
}

void executeTimer1ACompareMatch()
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

void executeTimer1BCompareMatch()
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
