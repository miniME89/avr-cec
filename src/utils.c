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
 * @file utils.c
 * @author Marcel
 * @brief
 *
 */

#include "includes/utils.h"
#include "includes/defines.h"
#include "includes/peripherals.h"
#include <stdlib.h>

FIFOBuffer* newBufferFIFO(uint8_t size)
{
    FIFOBuffer* buffer = malloc(sizeof(FIFOBuffer));
    buffer->data = malloc(size * sizeof(char));
    buffer->size = size;
    buffer->read = 0;
    buffer->write = 0;
    
    return buffer;
}

bool putFIFO(FIFOBuffer* buffer, char byte)
{
    uint8_t next = ((buffer->write + 1) & (buffer->size - 1));
    
    if (buffer->read == next)
    {
        return false;
    }
    
    buffer->data[buffer->write] = byte;
    buffer->write = next;
    
    return true;
}

bool getFIFO(FIFOBuffer* buffer, char* byte)
{
    if (buffer->read == buffer->write)
    {
        return false;
    }
    
    *byte = buffer->data[buffer->read];
    buffer->read = (buffer->read+1) & (buffer->size - 1);
    
    return true;
}

bool isEmptyFIFO(FIFOBuffer* buffer)
{
    return (buffer->read == buffer->write);
}
