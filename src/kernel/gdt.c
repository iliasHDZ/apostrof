#include "gdt.h"

seg_descriptor gdt[3];

extern void apply_gdt();//u32 base, u16 limit);

void gdt_register(u8 index, u32 base, u32 limit, u8 access, u8 flags) {
    seg_descriptor* d = &gdt[index];

    d->low_limit = limit & 0xffff;

    d->low_base  = base  & 0xffff;
    d->h1_base   = (base >> 16) & 0xff;
    d->h2_base   = (base >> 24) & 0xff;

    d->access  = access;
    d->flags   = flags | (limit >> 16);
}

void gdt_init() {
    gdt_register(0, 0, 0, 0, 0);

    // code segment
    gdt_register(1, 0x00000000, 0xffffffff, SEG_PRESENT | SEG_KERNEL | SEG_EXECUTABLE, FLG_GRANUL | FLG_32BIT);

    // data segment
    gdt_register(2, 0x00000000, 0xffffffff, SEG_PRESENT | SEG_KERNEL | SEG_WRITABLE,   FLG_GRANUL | FLG_32BIT);

    apply_gdt();//(u32)&gdt, sizeof(gdt) - 1);
}