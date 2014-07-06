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
 * @file debug.h
 * @author Marcel
 * @brief
 *
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "defines.h"
#include <inttypes.h>
#include <stdbool.h>

/**
 * Debug data.
 */
typedef struct DebugData
{
    char data[8];
    uint8_t size;
} DebugData;

#if DEBUG_AVR_CEC == 1
    /**
     * Initialize debug. Don't use this function, use the corresponding macro instead.
     */
    void _debugSetup(void);
    #define debugSetup() _debugSetup()

    /**
     * Add debug data. Don't use this function, use the corresponding macro instead.
     * @param data
     */
    void _debugPut(DebugData* data);
    #define debugPut(data) _debugPut(data)

    /**
     * Add debug char (1 byte). Don't use this function, use the corresponding macro instead.
     * @param c
     */
    void _debugPutChar(uint8_t c);
    #define debugPutChar(c) _debugPutChar(c)

    /**
     * Add debug word (2 bytes). Don't use this function, use the corresponding macro instead.
     * @param w
     */
    void _debugPutWord(uint16_t w);
    #define debugPutWord(w) _debugPutWord(w)

    /**
     * Add debug string.
     * @param str String of max 8 characters. Don't use this function, use the corresponding macro instead.
     */
    void _debugPutString(char* str);
    #define debugPutString(str) _debugPutString(str)

    /**
     * Get the next debug data element from the debug queue. Don't use this function, use the corresponding macro instead.
     * @param data
     * @return
     */
    bool _debugGet(DebugData* data);
    #define debugGet(data) _debugGet(data)
#else
    #define debugSetup()

    #define debugPut(data)

    #define debugPutChar(c)

    #define debugPutWord(w)

    #define debugPutString(str)

    #define debugGet(data) false
#endif

#endif
