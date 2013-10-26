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
 * @file usb.c
 * @author Marcel
 * @brief
 *
 */

#include "usb.h"
#include "defines.h"
#include "cec.h"
#include "usbdrv.h"
#include <util/delay.h>

static uchar messageBuffer[16];
static Message messageRead;
static Message messageWrite;

static uint8_t currentPosition;
static uint8_t bytesRemaining;

void initUsb()
{
    usbInit();

    usbDeviceDisconnect();

    for (uint8_t i = 0; i < 250; i++)
    {
        _delay_ms(2);
    }

    usbDeviceConnect();
}

void processUsb()
{
    usbPoll();
}

usbMsgLen_t usbFunctionSetup(uchar setupData[8])
{
    usbRequest_t *rq = (void*)setupData;
    switch (rq->bRequest)
    {
        case COMMAND_GET_MESSAGE:
            if (readMessage(&messageRead))
            {
                for (uint8_t i = 0; i < messageRead.size; i++)
                {
                    if (i == 0)
                    {
                        messageBuffer[i] = messageRead.header;
                    }
                    else if (i == 1)
                    {
                        messageBuffer[i] = messageRead.opcode;
                    }
                    else
                    {
                        messageBuffer[i] = messageRead.operands[i - 2];
                    }
                }
                usbMsgPtr = messageBuffer;

                return messageRead.size;
            }

            break;
        case COMMAND_PUT_MESSAGE:
            currentPosition = 0;
            bytesRemaining = rq->wLength.word;
            return USB_NO_MSG;

            break;
        case COMMAND_GET_CONFIG:

            return 0;

            break;
        case COMMAND_PUT_CONFIG:

            return 0;

            break;
    }

    return 0;
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
    if(len > bytesRemaining) {
        len = bytesRemaining;
    }

    bytesRemaining -= len;
    for(uint8_t i = 0; i < len; i++)
    {
        messageBuffer[currentPosition++] = data[i];
    }

    bool complete = bytesRemaining == 0;
    if (complete)
    {
        for (uint8_t i = 0; i < currentPosition; i++)
        {
            if (i == 0)
            {
                messageWrite.header = messageBuffer[i];
            }
            else if (i == 1)
            {
                messageWrite.opcode = messageBuffer[i];
            }
            else
            {
                messageWrite.operands[i - 2] = messageBuffer[i];
            }
        }

        messageWrite.size = currentPosition;
        writeMessage(messageWrite);
    }

    return complete;
}
