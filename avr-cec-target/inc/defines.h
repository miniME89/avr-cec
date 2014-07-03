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
 * @file defines.h
 * @author Marcel
 * @brief
 *
 */

#ifndef DEFINES_H_
#define DEFINES_H_

//==========================================
// Debug
//==========================================
#define DEBUG_AVR_CEC               1                                       //enable/disable debugging
#define DEBUG_QUEUE_SIZE            8                                       //size of the debug data queue (Note: each queue item will allocate 10 bytes of memory on the heap, if debugging is enabled)

//==========================================
// IO
//==========================================
/*
 * The INFO LED pin will be used to reflect the HIGH or LOW state of the CEC bus, so the user will get some feedback
 * that everything is working.
 */
#define INFO_LED_PORT               B
#define INFO_LED_PIN                PINB4

/*
 * The CEC output pin is used to only write to the CEC bus. This pin will be used to pull the bus LOW, which will result in a logical 0. To write a logical
 * 1 the pin will simply be disconnected, which results in the default HIGH state of the bus.
 */
#define CEC_OUTPUT_PORT           	D
#define CEC_OUTPUT_PIN            	PIND6

/*
 * The CEC input pin is used to only read the CEC bus.
 */
#define CEC_INPUT_PORT      		B
#define CEC_INPUT_PIN       		PINB0

//==========================================
// Timer
//==========================================
#define TIMER_PRESCALER             64                                      //prescaler (1, 8, 64, 256 or 1024)
#define TIMER_TICK_VALUE            (1000000 / (F_CPU/TIMER_PRESCALER))     //calculated time value of one timer tick (in us)

//==========================================
// CEC
//==========================================
/*
 * The size of the read and write queues for CEC messages. For each queue item 18 bytes of memory will be allocate on the heap on startup. The size of both
 * queues therefore depends on the size of the memory of the used microcontroller.
 */
#define CEC_READ_QUEUE_SIZE         8                                       //size of the cec messages read queue
#define CEC_WRITE_QUEUE_SIZE        8                                       //size of the cec messages write queue

/*
 * These settings will probably be removed in the future and hold in the memory for configuration at runtime.
 */
#define ENABLE_ASSERTION            0                                       //enable/disable assertion of data blocks send to the specified logical address
#define PHYSICAL_ADDRESS            0xFFFF
#define LOGICAL_ADDRESS             0x4

/*
 * The following defines will hold some important CEC timing values for the start bit. There is no need for changing there values. For further details check
 * the correspondent page in the HDMI CEC specification.
 */
#define START_BIT_T1                ((3.7 * 1000) / TIMER_TICK_VALUE)       //start bit: rising edge time t1 (in timer ticks => 3.7ms) (HDMI CEC specification p. 8)
#define START_BIT_T2                ((4.5 * 1000) / TIMER_TICK_VALUE)       //start bit: falling edge time t2 (in timer ticks => 4.5ms) (HDMI CEC specification p. 8)
#define START_BIT_TOLERANCE         ((0.2 * 1000) / TIMER_TICK_VALUE)       //start bit: rising/falling edge time tolerance (in timer ticks => 0.2ms) (HDMI CEC specification p. 8)

/*
 * The following defines will hold some important CEC timing values for the data bit. There is no need for changing there values. For further details check
 * the correspondent page in the HDMI CEC specification.
 */
#define DATA_BIT_LOGIC_0            ((1.5 * 1000) / TIMER_TICK_VALUE)       //data bit: rising edge time (in timer ticks => 1.5ms) to represent a logic 0 (HDMI CEC specification p. 9)
#define DATA_BIT_LOGIC_1            ((0.6 * 1000) / TIMER_TICK_VALUE)       //data bit: rising edge time (in timer ticks => 0.6ms) to represent a logic 1 (HDMI CEC specification p. 9)
#define DATA_BIT_FOLLOWING          ((2.4 * 1000) / TIMER_TICK_VALUE)       //data bit: start time (in timer ticks => 2.4ms) for a following data bit (HDMI CEC specification p. 9)
#define DATA_BIT_FOLLOWING_TIMEOUT  ((2.75 * 1000) / TIMER_TICK_VALUE)      //data bit: latest time for start of a following data bit (in timer ticks => 2.75ms) (HDMI CEC specification p. 9)
#define DATA_BIT_SAMPLE_TIME        ((1.05 * 1000) / TIMER_TICK_VALUE)      //data bit: sample time (in timer ticks => 1.05ms) (HDMI CEC specification p. 9)

/*
 * The following defines will hold some important CEC timing values for the signal free time. There is no need for changing there values. For further details check
 * the correspondent page in the HDMI CEC specification.
 */
#define SFT_PRESENT_INITIATOR       ((7 * 2.4 * 1000) / TIMER_TICK_VALUE)   //signal free time: present initiator sends another frame immediately after its previous frame (in timer ticks => 16.8ms) (HDMI CEC specification p. 15)
#define SFT_NEW_INITIATOR           ((5 * 2.4 * 1000) / TIMER_TICK_VALUE)   //signal free time: new initiator wants to send a frame (in timer ticks => 12ms) (HDMI CEC specification p. 15)
#define SFT_RETRANSMISSION          ((3 * 2.4 * 1000) / TIMER_TICK_VALUE)   //signal free time: retransmission after a previous unsuccessful attempt (in timer ticks => 7.2ms) (HDMI CEC specification p. 15)

//==========================================
// USB
//==========================================
#define COMMAND_GET_MESSAGE         1
#define COMMAND_PUT_MESSAGE         2
#define COMMAND_GET_CONFIG          3
#define COMMAND_PUT_CONFIG          4
#define COMMAND_GET_DEBUG           5

#endif
