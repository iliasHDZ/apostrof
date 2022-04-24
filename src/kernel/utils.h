#pragma once

typedef char  i8;
typedef short i16;
typedef int   i32;
typedef long  i64;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef float  f32;
typedef double f64;

void* memcpy(void* dst, const void* src, u32 size);

void* memset(void* ptr, int value, u32 size);

u8 cmpstr(const char* str1, const char* str2);

#define LOW16(v)  v & 0xFFFF
#define HIGH16(v) (v >> 16) & 0xFFFF