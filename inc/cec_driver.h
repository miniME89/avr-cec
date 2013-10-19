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
 * @file cec_driver.h
 * @author Marcel
 * @brief
 *
 */

#ifndef CEC_DRIVER_H_
#define CEC_DRIVER_H_

#include <inttypes.h>
#include <stdbool.h>

/**
 * CEC Message
 */
typedef struct Message
{
    char header;
    char opcode;
    char operands[14];
    uint8_t size;
} Message;

/**
 * Initialize the driver
 */
void initDriver(void);

/**
 * Process driver actions. An internal state machine takes care of the specific action which should be executed.
 */
void processDriver(void);

/**
 * Check whether there are events to process of the internal state machine. If this is the case, any delaying actions of
 * other modules should be canceled.
 * @return Returns true if there are unprocessed events.
 */
bool isEvent(void);

/**
 * Write a CEC message on the bus. The message will be put into the send queue and processed later.
 * @param message The CEC message to write.
 * @return Returns true if the message was successful put into a send queue.
 */
bool writeMessage(Message message);

/**
 * Read the next message from the receive queue of messages read from the bus.
 * @param message A pointer to a Message where the data should be copied.
 * @return Returns true if the message was successful read from the queue. Returns false if the receive queue is empty.
 */
bool readMessage(Message* message);

/**
 * Called when input capture interrupt is executed.
 */
void executeTimer1InputCapture(void);

/**
 * Called when compare match A is executed.
 */
void executeTimer1ACompareMatch(void);

/**
 * Called when compare match B is executed.
 */
void executeTimer1BCompareMatch(void);

#endif
