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
#include "debug.h"
#include "cec.h"
#include "usbdrv.h"
#include <util/delay.h>

static Message messageRead;
static Message messageWrite;
static DebugData debugData;

static char bufferData[16];
static uint8_t readCommand;
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
                        bufferData[i] = messageRead.header;
                    }
                    else if (i == 1)
                    {
                        bufferData[i] = messageRead.opcode;
                    }
                    else
                    {
                        bufferData[i] = messageRead.operands[i - 2];
                    }
                }
                usbMsgPtr = bufferData;

                return messageRead.size;
            }

        break;
        case COMMAND_PUT_MESSAGE:
            currentPosition = 0;
            bytesRemaining = rq->wLength.word;
            readCommand = COMMAND_PUT_MESSAGE;
            return USB_NO_MSG;

        break;
        case COMMAND_GET_CONFIG:

        break;
        case COMMAND_PUT_CONFIG:
            currentPosition = 0;
            bytesRemaining = rq->wLength.word;
            readCommand = COMMAND_PUT_CONFIG;
            return USB_NO_MSG;

        break;
        case COMMAND_GET_DEBUG:
            if (readDebug(&debugData))
            {
                for (uint8_t i = 0; i < debugData.size; i++)
                {
                    bufferData[i] = debugData.data[i];
                }
                usbMsgPtr = bufferData;

                return debugData.size;
            }
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
        bufferData[currentPosition++] = data[i];
    }

    bool complete = bytesRemaining == 0;
    if (complete)
    {
        switch (readCommand) {
            case COMMAND_PUT_MESSAGE:
                for (uint8_t i = 0; i < currentPosition; i++)
                {
                    if (i == 0)
                    {
                        messageWrite.header = bufferData[i];
                    }
                    else if (i == 1)
                    {
                        messageWrite.opcode = bufferData[i];
                    }
                    else
                    {
                        messageWrite.operands[i - 2] = bufferData[i];
                    }
                }

                messageWrite.size = currentPosition;
                writeMessage(messageWrite);
            break;
            case COMMAND_PUT_CONFIG:
                //...
            break;
        }
    }

    return complete;
}
