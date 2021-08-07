#pragma once

#include "utils.h"

#define VMEM_TABLE_ENTRIES 1024
#define VMEM_DIR_ENTRIES   1024

#define VMEM_KERNEL_PAGES  4096

#define VMEM_PRESENT  0x00000001
#define VMEM_WRITABLE 0x00000002

#define VMEM_USER     0x00000004
#define VMEM_KERNEL   0x00000000

#define VMEM_TASK_STACK 0x7fffffff

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

vmem* vmem_createMemory();

void vmem_switchMemory(vmem* virmem);

void vmem_enable();

void vmem_init();