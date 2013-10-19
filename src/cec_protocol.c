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
 * @file cec_protocol.c
 * @author Marcel
 * @brief
 *
 */

#include "includes/cec_protocol.h"
#include "includes/cec_driver.h"
#include "includes/peripherals.h"
#include <inttypes.h>

Message msgReadCec;
Message msgWriteCec;
Message msgReadUart;

void processProtocol(void)
{
    //try to get next CEC message from CEC read queue
    if (readMessage(&msgReadCec))
    {
        for (uint8_t i = 0; i < msgReadCec.size; i++)
        {
            if (i == 0)
            {
                uartSendChar(msgReadCec.header);
            }
            else if (i == 1)
            {
                uartSendChar(msgReadCec.opcode);
            }
            else
            {
                uartSendChar(msgReadCec.operands[i - 2]);
            }
        }
        uartSendChar('\n');
    }

    //try to get next char from UART read queue
    char c;
    if (uartReadChar(&c))
    {
        if (c == '\n')                              //EOM indicated by LF
        {
            writeMessage(msgReadUart);
            msgReadUart.size = 0;
        }
        else
        {
            if (msgReadUart.size == 0)
            {
                msgReadUart.header = c;
            }
            else if (msgReadUart.size == 1)
            {
                msgReadUart.opcode = c;
            }
            else
            {
                msgReadUart.operands[msgReadUart.size - 2] = c;
            }

            msgReadUart.size++;
        }
    }
}
