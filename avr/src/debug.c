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
#include "utils.h"

#if DEBUG_AVR_CEC == 1
    static QueueDebug* debugQueue;
#endif

void initDebug()
{
    #if DEBUG_AVR_CEC == 1
        debugQueue = newQueueDebug(16);
    #endif
}

void debug(DebugData data)
{
    #if DEBUG_AVR_CEC == 1
        putDebug(debugQueue, data);
    #endif
}

void debug_char(uint8_t c)
{
    #if DEBUG_AVR_CEC == 1
        DebugData data;
        data.data[0] = c;
        data.size = 1;
        putDebug(debugQueue, data);
    #endif
}

void debug_word(uint16_t w)
{
    #if DEBUG_AVR_CEC == 1
        DebugData data;
        data.data[0] = (uint8_t)(w >> 8);
        data.data[1] = (uint8_t)w;
        data.size = 2;
        putDebug(debugQueue, data);
    #endif
}

void debug_string(char* str)
{
    #if DEBUG_AVR_CEC == 1
        DebugData data;
        for (data.size = 0; *str; str++, data.size++)
        {
            data.data[data.size] = *str;
        }
        putDebug(debugQueue, data);
    #endif
}

bool readDebug(DebugData* data)
{
    #if DEBUG_AVR_CEC == 1
        return getDebug(debugQueue, data);
    #else
        return false;
    #endif
}
