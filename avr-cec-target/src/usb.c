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

static CECMessage messageRead;
static CECMessage messageWrite;
static DebugData debugData;

static char bufferData[16];
static uint8_t readCommand;
static uint8_t currentPosition;
static uint8_t bytesRemaining;

void usbSetup()
{
    usbInit();

    usbDeviceDisconnect();

    for (uint8_t i = 0; i < 250; i++)
    {
        _delay_ms(2);
    }

    usbDeviceConnect();
}

void usbProcess()
{
    usbPoll();
}

usbMsgLen_t usbFunctionSetup(uchar setupData[8])
{
    usbRequest_t *rq = (void*)setupData;

    switch (rq->bRequest)
    {
        case COMMAND_GET_MESSAGE:
            if (readCECMessage(&messageRead))
            {
                for (uint8_t i = 0; i < messageRead.size; i++)
                {
                    bufferData[i] = messageRead.data[i];
                }
                usbMsgPtr = (usbMsgPtr_t)bufferData;

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
            if (debugGet(&debugData))
            {
                for (uint8_t i = 0; i < debugData.size; i++)
                {
                    bufferData[i] = debugData.data[i];
                }
                usbMsgPtr = (usbMsgPtr_t)bufferData;

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
                    messageWrite.data[i] = bufferData[i];
                }

                messageWrite.size = currentPosition;
                writeCECMessage(&messageWrite);
            break;
            case COMMAND_PUT_CONFIG:
                //...
            break;
        }
    }

    return complete;
}
