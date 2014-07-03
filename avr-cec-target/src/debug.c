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
 * @file debug.c
 * @author Marcel
 * @brief
 *
 */

#include "debug.h"
#include "defines.h"
#include "queue.h"

#if DEBUG_AVR_CEC == 1

//==========================================
// Declarations
//==========================================
static Queue* debugQueue;

//==========================================
// Definitions
//==========================================
void _debugSetup()
{
    debugQueue = queueCreate(DEBUG_QUEUE_SIZE, sizeof(DebugData));
}

void _debugPut(DebugData* data)
{
    queuePut(debugQueue, data);
}

void _debugPutChar(uint8_t c)
{
    DebugData data;
    data.data[0] = c;
    data.size = 1;

    queuePut(debugQueue, &data);
}

void _debugPutWord(uint16_t w)
{
    DebugData data;
    data.data[0] = (uint8_t) (w >> 8);
    data.data[1] = (uint8_t) w;
    data.size = 2;

    queuePut(debugQueue, &data);
}

void _debugPutString(char* str)
{
    DebugData data;
    for (data.size = 0; *str; str++, data.size++)
    {
        data.data[data.size] = *str;
    }

    queuePut(debugQueue, &data);
}

bool _debugGet(DebugData* data)
{
    return queueGet(debugQueue, data);
}

#endif
