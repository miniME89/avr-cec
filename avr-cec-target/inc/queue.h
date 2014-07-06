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
 * @file queue.h
 * @author Marcel
 * @brief
 *
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <inttypes.h>
#include <stdbool.h>

/**
 * The queue structure representing a FIFO queue for generic data.
 */
typedef struct Queue {
    void** element;         //!< A pointer to each item of the queue. It is therefore an array of void pointers.
    uint8_t num;            //!< Number of the elements in the queue.
    uint8_t size;           //!< Size of each individual element in the queue.
    uint8_t read;           //!< Position of the next element in the queue returned by the getQueue function.
    uint8_t write;          //!< Position of the next empty slot the putQueue function will write a new element.
} Queue;

/**
 * Create a new FIFO queue. This will allocate the necessary memory. Due to the runtime allocation this function takes a decent amount of time and should be called
 * at the start of the program in some initialization process.
 * @param size The maximum size of elements in the queue.
 * @param sizeEach The size each individual element in the queue.
 * @return Returns a handle for the queue created.
 */
Queue* queueCreate(uint8_t num, uint8_t size);

/**
 * Delete the FIFO queue. This will free any allocated memory.
 * @param queue The queue handle.
 */
void queueDelete(Queue* queue);

/**
 * Put a new element to the end of the queue.
 * @param queue The queue handle.
 * @param element The pointer to the data structure which should be added to the queue.
 * @return Returns true if the element was successfully added to the queue. If the queue is full the function will return false.
 */
bool queuePut(Queue* queue, void* element);

/**
 * Get the next element from the beginning of the queue.
 * @param queue The queue handle.
 * @param element The pointer to the data structure where the element will be copied.
 * @return Returns true if the element was successfully read from the queue. If the queue is empty and can't return any element the function will return false.
 */
bool queueGet(Queue* queue, void* element);

/**
 * Check if the queue is empty.
 * @param queue The queue handle.
 * @return Returns true if the queue is empty.
 */
bool queueIsEmpty(Queue* queue);

/**
 * Clear the queue.
 * @param queue The queue handle.
 */
void queueClear(Queue* queue);

#endif
