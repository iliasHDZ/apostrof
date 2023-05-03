#pragma once

#include "../utils.h"

#define KERNEL_CS 0x08

typedef struct {
    u16 loffset;
    u16 sel;
    u8  always0;
    u8  flags; 
    u16 hoffset;
} __attribute__((packed)) idt_gate;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) idt_pointer;

#define IDT_ENTRIES 256
extern idt_gate    idt[IDT_ENTRIES];
extern idt_pointer idt_ptr;

void idt_setGate(int n, u32 handler);

void idt_init();