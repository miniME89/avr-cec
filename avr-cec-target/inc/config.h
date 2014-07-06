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
 * @file config.h
 * @author Marcel
 * @brief
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

//==========================================
// Debug
//==========================================
/*
 * enable/disable debugging
 */
#define DEBUG_AVR_CEC               1

/*
 * size of the debug data queue (Note: each queue item will allocate 10 bytes of memory on the heap, if debugging is enabled)
 */
#define DEBUG_QUEUE_SIZE            8


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
#define CEC_OUTPUT_PORT             D
#define CEC_OUTPUT_PIN              PIND6

/*
 * The CEC input pin is used to only read the CEC bus.
 */
#define CEC_INPUT_PORT              B
#define CEC_INPUT_PIN               PINB0


//==========================================
// Timer
//==========================================
/*
 * prescaler (1, 8, 64, 256 or 1024)
 */
#define TIMER_PRESCALER             64


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


//==========================================
// USB
//==========================================


#endif
