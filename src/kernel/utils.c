#include "utils.h"

void* memcpy(void* dst, const void* src, u32 size) {
    for (u32 i = 0; i < size; i++)
        ((u8*)dst)[i] = ((u8*)src)[i];

    return dst;
}

void* memset(void* ptr, int value, u32 size) {
    for (u32 i = 0; i < size; i++)
        ((u8*)ptr)[i] = (u8)value;

    return ptr;
}