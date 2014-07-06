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
 * @file cec.c
 * @author Marcel
 * @brief
 *
 */

#include "cec.h"
#include "defines.h"
#include "peripherals.h"
#include "queue.h"
#include "debug.h"

//==========================================
// Declarations
//==========================================
/**
 * Typedef for a function pointer representing a state.
 */
typedef void (*StatePointer)(void);

/**
 *
 */
typedef enum StateMachine
{
    SM_MAIN = 0,       //!< SM_MAIN
    SM_READ = 1,       //!< SM_READ
    SM_WRITE = 2,      //!< SM_WRITE
    SM_READSTARTBIT = 3//!< SM_READSTARTBIT
} StateMachine;

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
static void setState(StateMachine stateMachine, StatePointer state);

/**
 *
 * @param stateMachine
 */
static void executeStateMachine(StateMachine stateMachine);

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

/**
 * Called when input capture interrupt is executed.
 */
static void callbackInputCapture(void);

/**
 * Called when compare match A is executed.
 */
static void callbackTimerA(void);

/**
 * Called when compare match B is executed.
 */
static void callbackTimerB(void);

/**
 * Check if there is a triggered timer A event. This will consume the event if there is one.
 * @return Returns true if there is a triggered timer A event.
 */
static bool isEventTriggeredTimerA(bool consume);

/**
 * Check if there is a triggered timer B event. This will consume the event if there is one.
 * @return Returns true if there is a triggered timer B event.
 */
static bool isEventTriggeredTimerB(bool consume);

/**
 * Check if there is a input toggled event. This will consume the event if there is one.
 * @return Returns true if there is a input toggled event.
 */
static bool isEventInputToggled(bool consume);

/**
 *
 * @return
 */
bool isEventFallingEdge(bool consume);

/**
 *
 * @return
 */
bool isEventRisingEdge(bool consume);

/**
 * Check if there is a state transition in event. This will consume the event if there is one.
 * @return Returns true if there is a state transition in event.
 */
static bool isEventTransistionIn(StateMachine stateMachine, bool consume);

/**
 * Check if there is a state transition out event. This will consume the event if there is one.
 * @return Returns true if there is a state transition out event.
 */
static bool isEventTransistionOut(StateMachine stateMachine, bool consume);

/**
 *
 */
static void stateMain_Initial(void);

/**
 *
 */
static void stateMain_Read(void);

/**
 *
 */
static void stateMain_Write(void);

/**
 *
 */
static void stateMain_Final(void);

/**
 *
 */
static void stateRead_Initial(void);

/**
 *
 */
static void stateRead_StartBit(void);

/**
 *
 */
static void stateRead_DataBits(void);

/**
 *
 */
static void stateRead_Final(void);

/**
 *
 */
static void stateWrite_Initial(void);

/**
 *
 */
static void stateWrite_SignalFreeTime(void);

/**
 *
 */
static void stateWrite_StartBits(void);

/**
 *
 */
static void stateWrite_DataBits(void);

/**
 *
 */
static void stateWrite_Final(void);

//==========================================
// Variables
//==========================================
static StatePointer currentState[4];

bool beginStartBit = false;

static bool eventTriggeredTimerA = false;
static bool eventTriggeredTimerB = false;
static bool eventToggledEdge = false;
static bool eventFallingEdge = false;
static bool eventRisingEdge = false;
static bool eventTransistionIn[4] = {true};
static bool eventTransistionOut[4] = {false};

static TimerOptions timerOptionsA = {.repeat = false, .reset = false};
static TimerOptions timerOptionsB = {.repeat = false, .reset = false};

static Queue* messageQueueRead;
static Queue* messageQueueWrite;
static CECMessage messageBufferWrite;
static CECMessage messageBufferRead;

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
void cecSetup(void)
{
    //create queues
    messageQueueRead = queueCreate(CEC_READ_QUEUE_SIZE, sizeof(CECMessage));
    messageQueueWrite = queueCreate(CEC_WRITE_QUEUE_SIZE, sizeof(CECMessage));

    //register callbacks
    registerCallbackInputCapture(callbackInputCapture);
    registerCallbackTimerA(callbackTimerA);
    registerCallbackTimerB(callbackTimerB);

    //set initial state
    setState(SM_MAIN, stateMain_Initial);
}

void cecProcess()
{
    executeStateMachine(SM_MAIN);
}

static void setState(StateMachine stateMachine, StatePointer state)
{
    currentState[stateMachine] = state;
    eventTransistionIn[stateMachine] = true;
    eventTransistionOut[stateMachine] = true;
}

static void executeStateMachine(StateMachine stateMachine)
{
    currentState[stateMachine]();
}

bool writeCECMessage(CECMessage* message)
{
    return queuePut(messageQueueWrite, message);
}

bool readCECMessage(CECMessage* message)
{
    return queueGet(messageQueueRead, message);
}

void setTimeout(uint16_t ticks, Timer timer, bool reset, bool repeat)
{
    if (timer == TIMER_A)
    {
        setValueTimerCompareMatch(TIMER_A, ticks);
        setInterruptTimerCompareMatch(TIMER_A, true);
        timerOptionsA.reset = reset;
        timerOptionsA.repeat = repeat;
        eventTriggeredTimerA = false;
    }
    else
    {
        setValueTimerCompareMatch(TIMER_B, ticks);
        setInterruptTimerCompareMatch(TIMER_B, true);
        timerOptionsB.reset = reset;
        timerOptionsB.repeat = repeat;
        eventTriggeredTimerB = false;
    }
}

void clearTimeout(Timer timer)
{
    setInterruptTimerCompareMatch(timer, false);
    if (timer == TIMER_A)
    {
        eventTriggeredTimerA = false;
    }
    else
    {
        eventTriggeredTimerB = false;
    }
}

void low()
{
    setOutCECLevel(LOW);
    shouldLevel = LOW;
}

void high()
{
    setOutCECLevel(HIGH);
    shouldLevel = HIGH;
}

bool verifyLevel()
{
    return (shouldLevel == lastEdgeLevel);
}

void callbackInputCapture()
{
    lastEdgeLevel = getInCECLevel();
    lastEdgeTicks = getTimerTicks();
    eventToggledEdge = true;
    if (lastEdgeLevel == HIGH)
    {
        eventRisingEdge = true;
    }
    else
    {
        eventFallingEdge = true;
    }
    setOutInfoLEDLevel(lastEdgeLevel);
}

void callbackTimerA()
{
    if (timerOptionsA.reset)
    {
        resetTimer();
    }

    if (!timerOptionsA.repeat)
    {
        //disable timer1 compare A interrupt
        setInterruptTimerCompareMatch(TIMER_A, false);
    }

    eventTriggeredTimerA = true;
}

void callbackTimerB()
{
    if (timerOptionsB.reset)
    {
        resetTimer();
    }

    if (!timerOptionsB.repeat)
    {
        //disable timer1 compare B interrupt
        setInterruptTimerCompareMatch(TIMER_B, false);
    }

    eventTriggeredTimerB = true;
}

/*
 * Events
 */
bool isEventTriggeredTimerA(bool consume)
{
    if (eventTriggeredTimerA)
    {
        eventTriggeredTimerA = (consume) ? false : true;

        return true;
    }

    return false;
}

bool isEventTriggeredTimerB(bool consume)
{
    if (eventTriggeredTimerB)
    {
        eventTriggeredTimerB = (consume) ? false : true;

        return true;
    }

    return false;
}

bool isEventInputToggled(bool consume)
{
    if (eventToggledEdge)
    {
        eventToggledEdge = (consume) ? false : true;
        eventFallingEdge = eventToggledEdge && eventFallingEdge;
        eventRisingEdge = eventToggledEdge && eventRisingEdge;

        return true;
    }

    return false;
}

bool isEventFallingEdge(bool consume)
{
    if (eventFallingEdge)
    {
        eventToggledEdge = (consume) ? false : true;
        eventFallingEdge = eventToggledEdge;

        return true;
    }

    return false;
}

bool isEventRisingEdge(bool consume)
{
    if (eventRisingEdge)
    {
        eventToggledEdge = (consume) ? false : true;
        eventRisingEdge = eventToggledEdge;

        return true;
    }

    return false;
}

bool isEventTransistionIn(StateMachine stateMachine, bool consume)
{
    if (eventTransistionIn[stateMachine])
    {
        eventTransistionIn[stateMachine] = (consume) ? false : true;

        return true;
    }

    return false;
}

bool isEventTransistionOut(StateMachine stateMachine, bool consume)
{
    if (eventTransistionOut[stateMachine])
    {
        eventTransistionOut[stateMachine] = (consume) ? false : true;

        return true;
    }

    return false;
}

/*
 * state machine:     SM_MAIN
 * states:            Initial, Read, Write, Final
 * description:        
 */
void stateMain_Initial()
{
    CECMessage message;
    message.data[0] = 0x05;
    message.data[1] = 0x44;
    message.data[2] = 0x43;
    message.size = 3;
    writeCECMessage(&message);

    setState(SM_MAIN, stateMain_Read);
}

void stateMain_Read()
{
    //transition in event
    if (isEventTransistionIn(SM_MAIN, true))
    {
        setState(SM_READ, stateRead_Initial);
    }

    //execute sub state machine
    executeStateMachine(SM_READ);

    //transition out event
    if (isEventTransistionOut(SM_MAIN, true))
    {

    }
}

void stateMain_Write()
{
    //transition in event
    if (isEventTransistionIn(SM_MAIN, true))
    {
        setState(SM_WRITE, stateWrite_Initial);
    }

    //execute sub state machine
    executeStateMachine(SM_WRITE);

    //transition out event
    if (isEventTransistionOut(SM_MAIN, true))
    {

    }
}

void stateMain_Final()
{

}

/*
 * state machine:     SM_READ
 * states:            Initial, StartBit, DataBit, Final
 * description:
 */
void stateRead_Initial()
{
    setState(SM_READ, stateRead_StartBit);
}

void stateRead_StartBit()
{
    //transition in event
    if (isEventTransistionIn(SM_READ, true))
    {
        beginStartBit = false;
    }

    //falling edge event: possible beginning (T0) or end (T2) of a start bit
    if (isEventFallingEdge(true))
    {
        //no possible beginning of start bit already found? (T0)
        if (!beginStartBit)
        {
            resetTimer();
            beginStartBit = true;
        }
        //possible end of a start bit (T2)
        else
        {
            //invalid time and therefore not a start bit?
            if ((START_BIT_T2 + START_BIT_TOLERANCE) < lastEdgeTicks && lastEdgeTicks < (START_BIT_T2 - START_BIT_TOLERANCE))
            {
                setState(SM_READ, stateRead_StartBit);
            }
            //valid time for end of a start bit
            else
            {
                setState(SM_READ, stateRead_DataBits);
            }
        }
    }

    //rising edge event: possible rising edge of a start bit (T1)
    if (isEventRisingEdge(true))
    {
        //invalid time and therefore not a start bit?
        if ((START_BIT_T1 + START_BIT_TOLERANCE) < lastEdgeTicks && lastEdgeTicks < (START_BIT_T1 - START_BIT_TOLERANCE))
        {
            setState(SM_READ, stateRead_StartBit);
        }
    }

    //is message write buffer empty?
    if (messageBufferWrite.size == 0)
    {
        //try to get next message from write queue
        queueGet(messageQueueWrite, &messageBufferWrite);
    }
    //a message is in write buffer
    else if (!beginStartBit)
    {
        setState(SM_READ, stateRead_Final);
    }

    //transition out event
    if (isEventTransistionOut(SM_READ, true))
    {

    }
}

void stateRead_DataBits()
{
    //transition in event
    if (isEventTransistionIn(SM_READ, true))
    {
        messageBufferRead.size = 0;
        bitCounter = 0;
        byteCounter = 0;
        EOM = LOW;
        ACK = HIGH;

        eventToggledEdge = true;
        eventFallingEdge = true;
    }

    //falling edge event: begin of next data bit
    if (isEventFallingEdge(true))
    {
        //set timer to sample data bit at the save sample period (1.05ms)
        resetTimer();
        setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_A, false, false);

        //is this bit an ACK bit?
        if (bitCounter == 9)
        {
            //is this message directed at this destination address?
            if ((messageBufferRead.data[0] & 0x0F) == LOGICAL_ADDRESS)
            {
                //assert bit by pulling line low and set timeout when to pull back high again
                #if ENABLE_ASSERTION == 1
                    low();
                    setTimeout(DATA_BIT_LOGIC_0, TIMER_B, false, false);
                #endif
            }
        }
    }

    //rising edge event: end of data bit
    if (isEventRisingEdge(true))
    {
        //is this the end of the message?
        if (EOM == HIGH)
        {
            clearTimeout(TIMER_A);
            clearTimeout(TIMER_B);

            //put the read message from the buffer into the read message queue
            messageBufferRead.size = byteCounter;
            queuePut(messageQueueRead, &messageBufferRead);

            setState(SM_READ, stateRead_StartBit);
        }
    }

    //timer A event: on sample time (1.05ms) after begin of every bit to read the logic level
    if (isEventTriggeredTimerA(true))
    {
        //is information bit? (bit 0 to 7)
        if (bitCounter <= 7)
        {
            //save the bit in the message read buffer
            messageBufferRead.data[byteCounter] = (messageBufferRead.data[byteCounter] << 1) | lastEdgeLevel;
        }

        //is last information bit of data block? (bit 7)
        if (bitCounter == 7)
        {
            byteCounter++;

            //does the read message exceed the max message size of 16 blocks (16 x 10 bit)?
            if (byteCounter > 15)
            {
                //TODO error handling
                setState(SM_READ, stateRead_StartBit);
            }
        }
        //is EOM bit of data block? (bit 8)
        else if (bitCounter == 8)
        {
            EOM = lastEdgeLevel;
        }
        //is ACK bit of data block? (bit 9)
        else if (bitCounter == 9)
        {
            ACK = lastEdgeLevel;
        }

        bitCounter++;

        //end of data block? (10 bits)
        if (bitCounter > 9)
        {
            bitCounter = 0;
        }
    }

    //timer B event: pull line up again when data block was asserted
    if (isEventTriggeredTimerB(true))
    {
        //pull line back high again (because the data block was asserted by pulling the line low before)
        #if ENABLE_ASSERTION == 1
            debugPutChar('A');
            high();
        #endif
    }

    //transition out event
    if (isEventTransistionOut(SM_READ, true))
    {
        messageBufferRead.size = 0;
        bitCounter = 0;
        byteCounter = 0;
    }
}

void stateRead_Final()
{
    setState(SM_MAIN, stateMain_Write);
}

/*
 * state machine:     SM_WRITE
 * states:            Initial, SignalFreeTime, StartBit, DataBit, Final
 * description:        This state machine is a sub state machine.
 */
void stateWrite_Initial()
{
    setState(SM_WRITE, stateWrite_SignalFreeTime);
}

void stateWrite_SignalFreeTime()
{
    //transition in event
    if (isEventTransistionIn(SM_WRITE, true))
    {
        //TODO check for correct signal free time: SFT_PRESENT_INITIATOR, SFT_NEW_INITIATOR or SFT_RETRANSMISSION
        resetTimer();
        setTimeout(SFT_NEW_INITIATOR, TIMER_A, true, false);
    }

    //input toggle event: someone other tries to write on the bus
    if (isEventInputToggled(false))
    {
        //cancel the write action on the bus and read the signal
        clearTimeout(TIMER_A);
        setState(SM_WRITE, stateWrite_Final);
        debugPutString("S");
    }

    //timer A event: waited for signal free time without recognizing any signal on the bus
    if (isEventTriggeredTimerA(true))
    {
        setState(SM_WRITE, stateWrite_StartBits);
    }

    //transition out event
    if (isEventTransistionOut(SM_WRITE, true))
    {

    }
}

void stateWrite_StartBits()
{
    //transition in event
    if (isEventTransistionIn(SM_WRITE, true))
    {
        //begin of start bit by pulling line low
        low();
        resetTimer();

        //timeout for pulling line high again
        setTimeout(START_BIT_T1, TIMER_A, false, false);

        //timeout for end of start bit
        setTimeout(START_BIT_T2, TIMER_B, true, false);
    }

    //input toggle event: TODO description
    if (isEventInputToggled(true))
    {
        //TODO implement proper monitoring..
        if (!verifyLevel())                                         //monitor CEC line
        {
            //TODO error handling: unexpected HIGH/LOW impedance on line
            high();

            clearTimeout(TIMER_A);
            clearTimeout(TIMER_B);

            setState(SM_WRITE, stateWrite_Final);
            eventToggledEdge = true;

            debugPutString("L");
        }
    }

    //timer A event: pull line back high again
    if (isEventTriggeredTimerA(true))
    {
        high();
    }

    //timer B event: end of start bit
    if (isEventTriggeredTimerB(true))
    {
        setState(SM_WRITE, stateWrite_DataBits);
    }

    //transition out event
    if (isEventTransistionOut(SM_WRITE, true))
    {

    }
}

void stateWrite_DataBits()
{
    //transition in event
    if (isEventTransistionIn(SM_WRITE, true))
    {
        bitCounter = 0;
        byteCounter = 0;

        eventTriggeredTimerA = true;
    }

    //toggle input event: do nothing but consume the event
    if (isEventInputToggled(true))
    {

    }

    //timer A event: beginning of every bit to pull line LOW / when line needs to be pulled HIGH again (dependend of logic 1/0)
    if (isEventTriggeredTimerA(true))
    {
        //start of next data bit
        if (lastEdgeLevel == HIGH)
        {
            //end of data block?
            if (bitCounter >= 10)
            {
                //is there a following data block?
                if ((byteCounter + 1) < messageBufferWrite.size)
                {
                    bitCounter = 0;
                    byteCounter++;
                }
                //end of message
                else
                {
                    setState(SM_WRITE, stateWrite_Final);
                }
            }

            //information bit? (bit 0 to 7)
            if (bitCounter <= 7)
            {
                low();

                //sample information bit at save sample period for verification
                setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);

                bool dataBit;
                dataBit = (messageBufferWrite.data[byteCounter] & (0b10000000 >> bitCounter)) > 0;

                //is logical 1?
                if (dataBit)
                {
                    setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                }
                //is logical 0
                else
                {
                    setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
                }

                bitCounter++;
            }
            //EOM bit? (bit 8)
            else if (bitCounter == 8)
            {
                low();

                //sample EOM bit at save sample period for verification
                setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);

                //is there a following data block?
                if ((byteCounter + 1) < messageBufferWrite.size)
                {
                    //EOM = 0
                    setTimeout(DATA_BIT_LOGIC_0, TIMER_A, false, false);
                }
                //there is no following data block
                else
                {
                    //EOM = 1
                    setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);
                }

                bitCounter++;
            }
            //ACK bit? (bit 9)
            else if (bitCounter == 9)
            {
                low();

                setTimeout(DATA_BIT_LOGIC_1, TIMER_A, false, false);

                //sample ACK bit at save sample period for verification
                setTimeout(DATA_BIT_SAMPLE_TIME, TIMER_B, false, false);

                bitCounter++;
            }
        }
        //back to high impedance of data bit
        else
        {
            high();

            //set timer for following data bit
            setTimeout(DATA_BIT_FOLLOWING, TIMER_A, true, false);
        }
    }

    //timer B event: on sample time (1.05ms) after start of every bit to verify bit on line and check assertion of any ACK bits
    if (isEventTriggeredTimerB(true))
    {
        //is information bit or EOM bit? (bit 0 to 9)
        if (bitCounter < 10)
        {
            //monitor CEC line
            if (!verifyLevel())
            {
                //TODO error handling: unexpected HIGH/LOW impedance on line
                high();

                clearTimeout(TIMER_A);
                clearTimeout(TIMER_B);

                setState(SM_WRITE, stateWrite_Final);
                eventToggledEdge = true;

                debugPutString("L");
            }
        }
        //is ACK bit
        else
        {
            //ACK bit not asserted by follower?
            if (lastEdgeLevel == HIGH)
            {
                //TODO error handling: ACK bit not asserted by follower
                debugPutString("E");
            }
        }
    }

    //transition out event
    if (isEventTransistionOut(SM_WRITE, true))
    {
        messageBufferWrite.size = 0;
        bitCounter = 0;
        byteCounter = 0;
    }
}

void stateWrite_Final()
{
    setState(SM_MAIN, stateMain_Read);
}
