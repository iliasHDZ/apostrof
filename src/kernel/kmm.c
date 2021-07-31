#include "kmm.h"

mem_entry* entries = (mem_entry*)MEM_ENTRY_BASE;
int entry_count = 0;

void kmm_init() {
    entries[0] = (mem_entry){0, 0};
}

u8 kmm_addEntry(u32 base, u32 limit) {
    if (entry_count >= MEM_ENTRIES) return 0;

    int e = 0;
    
    while (entries[e].base != 0) {
        if (e >= MEM_ENTRIES) return 0;
        if (entries[e].base > base) break;
        e++;
    }

    for (int i = entry_count; i >= e; i--)
        entries[i] = entries[i - 1];

    entries[e] = (mem_entry){base, limit};
    entry_count++;

    return 1;
}

u8 kmm_removeEntry(u32 base) {
    int e = 0;

    while (entries[e].base != 0) {
        if (e >= MEM_ENTRIES) return 0;
        if (e >= entry_count) return 0;
        if (entries[e].base == base) break;
        e++;
    }

    for (int i = e; i < entry_count - 1; i++)
        entries[i] = entries[i + 1];

    entries[entry_count - 1] = (mem_entry){0, 0};
    entry_count--;

    return 1;
}

mem_entry* kmm_getEntry(u32 base) {
    int e = 0;

    while (entries[e].base != 0) {
        if (e >= MEM_ENTRIES) return 0;
        if (e >= entry_count) return 0;
        if (entries[e].base == base) return &entries[e];
        e++;
    }
}

int kmm_getEntryIndex(u32 base) {
    int e = 0;

    while (entries[e].base != 0) {
        if (e >= MEM_ENTRIES) return 0;
        if (e >= entry_count) return 0;
        if (entries[e].base == base) return e;
        e++;
    }
}

void* kmalloc(u32 size) {
    u32 last_limit = KHEAP_BASE;

    for (int i = 0; i < entry_count; i++) {
        mem_entry* e = &entries[i];

        if ((e->base - last_limit) >= size)
            if (!kmm_addEntry(last_limit, last_limit + size)) return (void*)0;
            else return (void*)last_limit;

        last_limit = e->limit;
    }

    if ((KHEAP_LIMIT - last_limit) >= size)
        if (!kmm_addEntry(last_limit, last_limit + size)) return (void*)0;
        else return (void*)last_limit;
    else
        return (void*)0;
}

void kfree(void* block) {
    kmm_removeEntry((u32) block);
}