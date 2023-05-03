#include "idt.h"

idt_gate    idt[IDT_ENTRIES];
idt_pointer idt_ptr;

void idt_setGate(int n, u32 handler) {
    idt[n].loffset = LOW16(handler);
    idt[n].sel     = KERNEL_CS;
    idt[n].always0 = 0;
    idt[n].flags   = 0x8E; 
    idt[n].hoffset = HIGH16(handler);
}

void idt_init() {
    idt_ptr.base  = (u32)&idt;
    idt_ptr.limit = IDT_ENTRIES * sizeof(idt_gate) - 1;

    __asm__ __volatile__("lidtl (%0)" : : "r" (&idt_ptr));
}