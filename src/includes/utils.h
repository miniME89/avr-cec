#ifndef UTILS_H_
#define UTILS_H_
#include <inttypes.h>
#include <stdbool.h>

typedef struct FIFOBuffer {
    char* data;
    uint8_t size;
    uint8_t read;
    uint8_t write;
} FIFOBuffer;

FIFOBuffer* newBufferFIFO(uint8_t size);
bool putFIFO(FIFOBuffer* buffer, char byte);
bool getFIFO(FIFOBuffer* buffer, char* byte);
bool isEmptyFIFO(FIFOBuffer* buffer);

#endif
