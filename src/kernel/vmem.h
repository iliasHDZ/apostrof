#pragma once

#include "utils.h"

#define VMEM_TABLE_ENTRIES 1024
#define VMEM_DIR_ENTRIES   1024

#define VMEM_KERNEL_PAGES  4096

#define VMEM_PRESENT  0x00000001
#define VMEM_WRITABLE 0x00000002

#define VMEM_USER     0x00000004
#define VMEM_KERNEL   0x00000000

#define VMEM_TASK_STACK 0xC0000000
#define VMEM_TASK_CODE  0x04000000
#define VMEM_TASK_HEAP  0x10000000

typedef struct vmem_page {
    u32 present    : 1;
    u32 writable   : 1;
    u32 user       : 1;
    u32 pwt        : 1;
    u32 pcd        : 1;
    u32 accessed   : 1;
    u32 dirty      : 1;
    u32 four_mb    : 1;
    u32 cpu_global : 1;
    u32 lv4_global : 1;
    u32 reserved   : 2;
    u32 frame      : 20;
} vmem_page;

typedef struct vmem_table_entry {
    u32 present    : 1;
    u32 writable   : 1;
    u32 user       : 1;
    u32 pwt        : 1;
    u32 pcd        : 1;
    u32 accessed   : 1;
    u32 intelresv  : 1;
    u32 four_mb    : 1;
    u32 global     : 1;
    u32 reserved   : 3;
    u32 frame      : 20;
} vmem_table_entry;

typedef struct vmem_table {
    vmem_page pages[VMEM_TABLE_ENTRIES];
} vmem_table;

typedef struct vmem {
    vmem_table_entry tables[VMEM_DIR_ENTRIES];
} vmem;

extern vmem* kernel_memory;

vmem* vmem_current();

u8 vmem_tablePresent(vmem* virmem, int table);

vmem_table* vmem_getTable(vmem* virmem, int table);

vmem_page* vmem_getPage(vmem* virmem, u32 page_addr);

u8 vmem_allocPage(vmem* virmem, u32 page_addr, u32 attr);

void vmem_freePage(vmem* virmem, u32 page_addr);

vmem* vmem_createMemory();

vmem* vmem_cloneKernelMemory();

void vmem_freeMemory(vmem* virmem);

void vmem_switchMemory(vmem* virmem);

vmem* vmem_createTaskMemory(u32 code_size, u32 stack_size);

void vmem_enable();

void vmem_init();