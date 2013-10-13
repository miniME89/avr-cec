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

void processProtocol(void)
{
    Message message;
    if (readMessage(&message))
    {
        for (uint8_t i = 0; i < message.size; i++)
        {
            if (i == 0)
            {
                uartSendChar(message.header);
            }
            else if (i == 1)
            {
                uartSendChar(message.opcode);
            }
            else
            {
                uartSendChar(message.operands[i - 2]);
            }
        }
        uartSendChar('\n');

        //test: display device as "PC" on TV
        if (message.header == 0x04 && message.opcode == 0x46)
        {
            Message m;
            m.header = 0x40;
            m.opcode = 0x47;
            m.operands[0] = 0x50;
            m.operands[1] = 0x43;
            m.size = 4;
            writeMessage(m);
        }
    }
}