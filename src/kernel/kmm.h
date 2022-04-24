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

typedef struct {
    void** buffer;

    u32    count;
    u32    size;
} PARRAY;

typedef struct {
    void** buffer;

    u32    count;
    u32    size;
    u32    capacity;
} ARRAY;

void kmm_init();

int parray(PARRAY* a, u32 size);

int parray_push(PARRAY* a, void* t);

int parray_remove(PARRAY* a, void* ptr);

void* parray_get(PARRAY* a, u32 i);

void parray_free(PARRAY* a);

int array(ARRAY* a, u32 size, u32 capacity);

int array_push(ARRAY* a, void* ptr_to_element);

int array_insert(ARRAY* a, void* ptr_to_element, u32 index);

int array_remove(ARRAY* a, u32 index);

int array_get(ARRAY* a, u32 index, void* dst);

void array_free(ARRAY* a);

void* memcpy(void* dst, const void* src, u32 size);

void* memset(void* ptr, int value, u32 size);

void* kmalloc(u32 size);

void* kmalloc_pa(u32 size);

void* krealloc(void* ptr, u32 size);

void kfree(void* block);