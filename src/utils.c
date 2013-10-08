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
