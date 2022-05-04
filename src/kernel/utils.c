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

u8 cmpstr(const char* str1, const char* str2) {
    int i = 0;

    while(str1[i] != 0 && str2[i] != 0) {
        if (str1[i] != str2[i]) return 0;
        i++;    
    }

    if (str1[i] != str2[i]) return 0;
    return 1;
}

u32 strlen(const char* str) {
    u32 i = 0;
    while(str[i] != 0) i++;

    return i;
}