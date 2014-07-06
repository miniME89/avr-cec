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

#include "queue.h"
#include "defines.h"
#include "peripherals.h"
#include "debug.h"
#include <stdlib.h>

//==========================================
// Definitions
//==========================================
Queue* queueCreate(uint8_t num, uint8_t size)
{
    //allocate memory for the queue structure itself
    Queue* queue = malloc(sizeof(Queue));

    //allocate memory for a pointer to each item of the queue
    queue->element = malloc(num * sizeof(void*));

    queue->num = num;
    queue->size = size;
    queue->read = 0;
    queue->write = 0;

    //allocate memory for each element
    for (uint8_t i = 0; i < num; i++)
    {
        queue->element[i] = malloc(size);
        if (queue->element[i] == NULL)
        {
            return NULL;
        }
    }

    return queue;
}

void queueDelete(Queue* queue)
{
    //free memory for each element
    for (uint8_t i = 0; i < queue->num; i++)
    {
        free(queue->element[i]);
    }

    //free memory for the element pointer
    free(queue->element);

    //free memory for the queue structure itself
    free(queue);
}

bool queuePut(Queue* queue, void* element)
{
    uint8_t next = ((queue->write + 1) & (queue->num - 1));

    if (queue->read == next)
    {
        return false;
    }

    char* dst = queue->element[queue->write];
    char* src = element;
    uint8_t num = queue->size;
    while (num--)
    {
        *dst++ = *src++;
    }

    queue->write = next;

    return true;
}

bool queueGet(Queue* queue, void* element)
{
    if (queue->read == queue->write)
    {
        return false;
    }

    char* dst = element;
    char* src = queue->element[queue->read];
    uint8_t num = queue->size;
    while (num--)
    {
        *dst++ = *src++;
    }

    queue->read = (queue->read+1) & (queue->num - 1);

    return true;
}

bool queueIsEmpty(Queue* queue)
{
    return (queue->read == queue->write);
}

void queueClear(Queue* queue)
{
    queue->read = 0;
    queue->write = 0;
}
