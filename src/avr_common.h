#pragma once

#include <inttypes.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

static inline void ak_memset(void *tgt, unsigned char val, unsigned int size)
{
    for (int i = 0; i < size; ++i)
    {
        ((char *)tgt)[i] = val;
    }
}

#define Min(a, b) ((a) <= (b) ? (a) : (b))
#define Max(a, b) ((a) >= (b) ? (a) : (b))