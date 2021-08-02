#pragma once

#include "utils.h"

#define MEM_ENTRY_BASE 0x00080000
#define MEM_ENTRIES    0xffff

#define KHEAP_BASE     0x00100000
#define KHEAP_LIMIT    0x01000000

typedef struct {
    u32 base;
    u32 limit;
} mem_entry;

void kmm_init();

void* memcpy(void* dst, const void* src, u32 size);

void* kmalloc(u32 size);

void* krealloc(void* ptr, u32 size);

void kfree(void* block);