#include "vmem.h"
#include "kmm.h"

#include "vga.h"

u8* vmem_bitmap      = 0;
u32 vmem_bitmapCount = 0;

vmem* vmem_current   = 0;
vmem* kernel_memory  = 0;

#define DEVIDE_CEIL(a, b) a / b + (a % b != 0)

u8 vmem_tablePresent(vmem* virmem, int table) {
    return virmem->tables[table].present;
}

vmem_table* vmem_getTable(vmem* virmem, int table) {
    return (vmem_table*)(virmem->tables[table].frame << 12);
}

vmem_table* vmem_allocTable() {
    vmem_table* table = kmalloc_pa(sizeof(vmem_table));
    if (table == 0) return 0;

    memset(table, 0, sizeof(vmem_table));

    return table;
}

u8 vmem_setTable(vmem* virmem, int i, vmem_table* table) {
    if (table == 0) virmem->tables[i] = (vmem_table_entry){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    virmem->tables[i] = (vmem_table_entry){ 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, (u32)table >> 12 };
    return 0;
}

u8 vmem_createTable(vmem* virmem, int i) {
    if (vmem_tablePresent(virmem, i)) return 0;

    vmem_table* table = vmem_allocTable();
    if (table == 0) return 1;
    
    return vmem_setTable(virmem, i, table);
}

u8 vmem_createPage(vmem_table* table, int page, u32 pys_addr, u32 attr) {
    if (table->pages[page].present) return 0;

    table->pages[page].present  = attr & VMEM_PRESENT  != 0;
    table->pages[page].writable = attr & VMEM_WRITABLE != 0;
    table->pages[page].user     = attr & VMEM_USER     != 0;

    table->pages[page].frame = pys_addr >> 12;

    return 0;
}

u8 vmem_mapPage(vmem* virmem, u32 page_addr, u32 pys_addr, u32 attr) {
    u32 page_idx  = (page_addr >> 12) & 0x3FF;
    u32 table_idx = (page_addr >> 22) & 0x3FF;

    if (!vmem_tablePresent(virmem, table_idx))
        vmem_createTable(virmem, table_idx);

    vmem_table* table = vmem_getTable(virmem, table_idx);

    return vmem_createPage(table, page_idx, pys_addr, attr);
}

u32 vmem_allocFrame() {
    for (int i = 0; i < vmem_bitmapCount; i++) {
        u32 byte = i / 8;
        u32 bit  = i % 8;

        u8 value = vmem_bitmap[byte] & (1 << bit);

        if (value == 0) {
            vmem_bitmap[byte] |= 1 << bit;

            return (i + VMEM_KERNEL_PAGES) << 12;
        }
    }

    return 0;
}

u8 vmem_allocPage(vmem* virmem, u32 page_addr, u32 attr) {
    u32 pys_addr = vmem_allocFrame();
    if (pys_addr == 0) return 1;

    vmem_mapPage(virmem, page_addr, pys_addr, attr);

    return 0;
}

u8 vmem_initBitmap(u32 mem_pages) {
    u32 user_pages   = mem_pages - VMEM_KERNEL_PAGES;
    vmem_bitmapCount = user_pages;

    u32 bitmapSize = vmem_bitmapCount / 8 + 1;
    vmem_bitmap = kmalloc(bitmapSize);

    if (vmem_bitmap == 0) return 1;

    memset(vmem_bitmap, 0, bitmapSize);
    return 0;
}

vmem* vmem_createMemory() {
    vmem* virmem = kmalloc_pa(sizeof(vmem));
    if (virmem == 0) return 0;

    memset(virmem, 0, sizeof(virmem));
    return virmem;
}

vmem_table* vmem_cloneTable(vmem* src, int i) {
    if (!src->tables[i].present) return 0;

    vmem_table* table = vmem_allocTable();
    if (table == 0) return 0;

    memcpy(table, (void*)(src->tables[i].frame << 12), sizeof(vmem_table));
    return table;
}

vmem* vmem_cloneMemory(vmem* src) {
    vmem* virmem = vmem_createMemory();

    for (int i = 0; i < 1024; i++)
        vmem_setTable(virmem, i, vmem_cloneTable(src, i));
    
    return virmem;
}

vmem* vmem_createTaskMemory(u32 code_size, u32 stack_size) {
    if (kernel_memory == 0) return 0;

    vmem* virmem = vmem_cloneMemory(kernel_memory);
    if (virmem == 0) return 0;

    u32 code_pages  = DEVIDE_CEIL(code_size, 4096);
    u32 stack_pages = DEVIDE_CEIL(stack_size, 4096);

    u32 code_base_page  = 0x4000000 >> 12;
    u32 stack_base_page = VMEM_TASK_STACK >> 12;

    for (int i = 0; i < code_pages; i++)
        vmem_allocPage(virmem, (code_base_page + i) << 12, VMEM_PRESENT | VMEM_WRITABLE | VMEM_USER);
        
    for (int i = 0; i < stack_pages; i++)
        vmem_allocPage(virmem, (stack_base_page - i) << 12, VMEM_PRESENT | VMEM_WRITABLE | VMEM_USER);

    return virmem;
}

void vmem_switchMemory(vmem* virmem) {
    vmem_current = virmem;
    asm volatile("mov %0, %%cr3":: "r"(virmem));
}

void vmem_enable() {
    u32 cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void vmem_init() {
    vmem_initBitmap(1048576);

    kernel_memory = vmem_createMemory();

    for (u32 i = 0; i < VMEM_KERNEL_PAGES; i++)
        vmem_mapPage(kernel_memory, i * 4096, i * 4096, VMEM_PRESENT | VMEM_KERNEL);

    vmem_switchMemory(kernel_memory);
    vmem_enable();
}