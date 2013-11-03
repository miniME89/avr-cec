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
 * @file utils.h
 * @author Marcel
 * @brief
 *
 * TODO implement generic queue
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "cec.h"
#include "debug.h"
#include <inttypes.h>
#include <stdbool.h>

typedef struct Queue {
    void** data;
    uint8_t size;
    uint8_t sizeEach;
    uint8_t read;
    uint8_t write;
} Queue;

Queue* newQueue(uint8_t size, uint8_t sizeEach);
bool putQueue(Queue* queue, void* data);
bool getQueue(Queue* queue, void* data);
bool isEmptyQueue(Queue* queue);


#endif