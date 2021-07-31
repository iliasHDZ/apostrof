#pragma once

#include "utils.h"

#define SEG_ACCESSED   0x01
#define SEG_WRITABLE   0x02
#define SEG_EXPDOWN    0x04
#define SEG_EXECUTABLE 0x08

#define SEG_KERNEL     0x00
#define SEG_USER       0x50
#define SEG_PRESENT    0x80

#define FLG_GRANUL     0x80
#define FLG_32BIT      0x40

typedef struct {
    u16 low_limit;
    u16 low_base;

    u8  h1_base;
    u8  access;
    u8  flags;
    u8  h2_base;
} seg_descriptor;

void gdt_init();
