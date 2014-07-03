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
 * @file cec.h
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
 * data[0] = header
 * data[1] = opcode
 * data[2-14] = operands
 */
typedef struct CECMessage
{
    char data[16];
    uint8_t size;
} CECMessage;

/**
 * Initialize the driver
 */
void cecSetup(void);

/**
 * Process driver actions. An internal state machine takes care of the specific action which should be executed.
 */
void cecProcess(void);

/**
 * Write a CEC message on the bus. The message will be put into the send queue and processed later.
 * @param message The CEC message to write.
 * @return Returns true if the message was successful put into a send queue.
 */
bool writeCECMessage(CECMessage* message);

/**
 * Read the next message from the receive queue of messages read from the bus.
 * @param message A pointer to a Message where the data should be copied.
 * @return Returns true if the message was successful read from the queue. Returns false if the receive queue is empty.
 */
bool readCECMessage(CECMessage* message);

/**
 * Called when input capture interrupt is executed.
 */
void executeTimerInputCapture(void);

/**
 * Called when compare match A is executed.
 */
void executeTimerACompareMatch(void);

/**
 * Called when compare match B is executed.
 */
void executeTimerBCompareMatch(void);

#endif
