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
QueueDebug* newQueueDebug(uint8_t size)
{
    QueueDebug* queue = malloc(sizeof(QueueDebug));
    queue->data = malloc(size * sizeof(DebugData));
    queue->size = size;
    queue->read = 0;
    queue->write = 0;
    
    return queue;
}

bool putDebug(QueueDebug* queue, DebugData data)
{
    uint8_t next = ((queue->write + 1) & (queue->size - 1));
    
    if (queue->read == next)
    {
        return false;
    }
    
    queue->data[queue->write] = data;
    queue->write = next;
    
    return true;
}

bool getDebug(QueueDebug* queue, DebugData* data)
{
    if (queue->read == queue->write)
    {
        return false;
    }
    
    *data = queue->data[queue->read];
    queue->read = (queue->read+1) & (queue->size - 1);
    
    return true;
}

bool isEmptyQueueDebug(QueueDebug* queue)
{
    return (queue->read == queue->write);
}

MessageQueue* newQueueMessage(uint8_t size)
{
    MessageQueue* queue = malloc(sizeof(MessageQueue));
    queue->data = malloc(size * sizeof(Message));
    queue->size = size;
    queue->read = 0;
    queue->write = 0;

    return queue;
}

bool putMessage(MessageQueue* queue, Message message)
{
    uint8_t next = ((queue->write + 1) & (queue->size - 1));

    if (queue->read == next)
    {
        debug_string("O");
        return false;
    }

    queue->data[queue->write] = message;
    queue->write = next;

    return true;
}

bool getMessage(MessageQueue* queue, Message* message)
{
    if (queue->read == queue->write)
    {
        return false;
    }

    *message = queue->data[queue->read];
    queue->read = (queue->read+1) & (queue->size - 1);

    return true;
}

bool isEmptyQueueMessage(MessageQueue* queue)
{
    return (queue->read == queue->write);
}
