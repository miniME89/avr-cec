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
 * Initialize debug. Don't use this function, use the macro "initDebug" instead.
 */
void _initDebug();

/**
 * Add debug data. Don't use this function, use the macro "debug" instead.
 * @param data
 */
void _debug(DebugData* data);

/**
 * Add debug char (1 byte). Don't use this function, use the macro "debug_char" instead.
 * @param c
 */
void _debug_char(uint8_t c);

/**
 * Add debug word (2 bytes). Don't use this function, use the macro "debug_word" instead.
 * @param w
 */
void _debug_word(uint16_t w);

/**
 * Add debug string. Don't use this function, use the macro "debug_string" instead.
 * @param str String of max 8 characters.
 */
void _debug_string(char* str);

/**
 * Get the next debug data from the debug queue. Don't use this function, use the macro "readDebugData" instead.
 * @param data
 * @return
 */
bool _readDebugData(DebugData* data);
#endif

//macros
#if DEBUG_AVR_CEC == 1
    #define initDebug() _initDebug()
    #define debug(data) _debug(data)
    #define debug_char(c) _debug_char(c)
    #define debug_word(w) _debug_word(w)
    #define debug_string(str) _debug_string(str)
    #define readDebugData(data) _readDebugData(data)
#else
    #define initDebug()
    #define debug(data)
    #define debug_char(c)
    #define debug_word(w)
    #define debug_string(str)
    #define readDebugData(data) false
#endif

#endif
