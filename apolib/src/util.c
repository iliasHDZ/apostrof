#include <apostrof.h>

unsigned int syscall(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx) {
    asm("int $0x1C\n" : "=a" (eax) : "a" (eax), "b" (ebx), "c" (ecx), "d" (edx));
    return eax;
}

void* memcpy(void* dst, const void* src, unsigned int size) {
    for (unsigned int i = 0; i < size; i++)
        ((unsigned char*)dst)[i] = ((unsigned char*)src)[i];

    return dst;
}

void* memset(void* ptr, int value, unsigned int size) {
    for (unsigned int i = 0; i < size; i++)
        ((unsigned char*)ptr)[i] = (unsigned char)value;

    return ptr;
}

unsigned int strlen(const char* str) {
    unsigned int i = 0;
    while(str[i] != 0) i++;

    return i;
}