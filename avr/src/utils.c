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

#include "utils.h"
#include "defines.h"
#include "peripherals.h"
#include "debug.h"
#include <stdlib.h>

//==========================================
// Definitions
//==========================================
Queue* newQueue(uint8_t size, uint8_t sizeEach)
{
    Queue* queue = malloc(sizeof(Queue));
    queue->data = malloc(size * sizeof(void*));
    queue->size = size;
    queue->sizeEach = sizeEach;
    queue->read = 0;
    queue->write = 0;

    for (uint8_t i = 0; i < size; i++)
    {
        queue->data[i] = malloc(sizeEach);
        if (queue->data[i] == NULL)
        {
            return NULL;
        }
    }

    return queue;
}

bool putQueue(Queue* queue, void* data)
{
    uint8_t next = ((queue->write + 1) & (queue->size - 1));

    if (queue->read == next)
    {
        debug_string("OV");
        return false;
    }

    char* dst = queue->data[queue->write];
    char* src = data;
    uint8_t num = queue->sizeEach;
    while (num--)
    {
        *dst++ = *src++;
    }

    queue->write = next;

    return true;
}

bool getQueue(Queue* queue, void* data)
{
    if (queue->read == queue->write)
    {
        return false;
    }

    char* dst = data;
    char* src = queue->data[queue->read];
    uint8_t num = queue->sizeEach;
    while (num--)
    {
        *dst++ = *src++;
    }

    queue->read = (queue->read+1) & (queue->size - 1);

    return true;
}

bool isEmptyQueue(Queue* queue)
{
    return (queue->read == queue->write);
}
