#include "kmm.h"

mem_entry* entries = (mem_entry*)MEM_ENTRY_BASE;
int entry_count = 0;

#define round_up(v, x) ((v / x + (v % x == 0 ? 0 : 1)) * x)

void kmm_init() {
    entries[0] = (mem_entry){0, 0};
}

int parray(PARRAY* a, u32 size) {
    a->count  = 0;
    a->size   = size;
    a->buffer = kmalloc(a->size * sizeof(u8*));

    if (a->buffer == 0) return -1;
    return 0;
}

int parray_push(PARRAY* a, void* ptr) {
    if (a->count >= a->size) {
        a->size  += 10;
        a->buffer = krealloc(a->buffer, a->size * sizeof(u8*));
        // Add check if krealloc failed
    }

    a->buffer[a->count++] = ptr;
    return 0;
}

int parray_remove(PARRAY* a, void* ptr) {
    for (int i = 0; i < a->count; i++)
        if (a->buffer[i] == ptr) {
            a->count--;
            for (; i < a->count; i++)
                a->buffer[i] = a->buffer[i + 1];

            return 0;
        }
    
    return -1;
}

void* parray_get(PARRAY* a, u32 i) {
    if (i < a->count) return a->buffer[i];
    else return 0;
}

void parray_free(PARRAY* a) {
    kfree(a->buffer);
}

int array(ARRAY* a, u32 size, u32 capacity) {
    a->count  = 0;
    a->size   = size;
    a->capacity = capacity;
    a->buffer = kmalloc(a->capacity * a->size);

    if (a->buffer == 0) return 0;
    return 1;
}

int array_push(ARRAY* a, void* ptr_to_element) {
    if (a->count >= a->capacity) {
        a->capacity += 10;
        a->buffer = krealloc(a->buffer, a->capacity * a->size);
        // Add check if krealloc failed
    }

    memcpy(a->buffer + a->count * a->size, ptr_to_element, a->size);
    a->count++;

    return 1;
}

int array_insert(ARRAY* a, void* ptr_to_element, u32 index) {
    if (a->count >= a->capacity) {
        a->capacity += 10;
        a->buffer = krealloc(a->buffer, a->capacity * a->size);
        // Add check if krealloc failed
    }

    memcpy(a->buffer + (index + 1) * a->size, a->buffer + index * a->size, (a->count++ - index) * a->size);
    memcpy(a->buffer + index * a->size, ptr_to_element, a->size);
    return 1;
}

int array_remove(ARRAY* a, u32 index) {
    if (index >= a->count || index < 0)
        return 0;

    memcpy(a->buffer + index * a->size, a->buffer + (index + 1) * a->size, (--a->count - index) * a->size);
    return 1;
}

int array_get(ARRAY* a, u32 index, void* dst) {
    if (index >= a->count || index < 0)
        return 0;

    memcpy(dst, a->buffer + index * a->size, a->size);
    return 1;
}

void array_free(ARRAY* a) {
    kfree(a->buffer);
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

    u8 found = 0;

    while (entries[e].base != 0) {
        if (e >= MEM_ENTRIES) return 0;
        if (e >= entry_count) return 0;
        if (entries[e].base == base) {
            found = 1;
            break;
        }
        e++;
    }

    if (!found) return 0;

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
        if (e >= MEM_ENTRIES) return -1;
        if (e >= entry_count) return -1;
        if (entries[e].base == base) return e;
        e++;
    }
}

void* krealloc(void* ptr, u32 size) {
    int idx = kmm_getEntryIndex((u32)ptr);
    if (idx == -1) return kmalloc(size);

    mem_entry* e = &entries[idx];

    if (idx + 1 >= entry_count)
        if ((KHEAP_LIMIT - e->base) >= size) {
            e->limit = e->base + size;
            return ptr;
        } else {
            u8* src = (u8*)(e->base);
            u32 len = e->limit - (u32)src;

            kmm_removeEntry((u32)src);

            void* ret = kmalloc(size);

            if (ret != 0)
                return memcpy(ret, src, size);
            else {
                kmm_addEntry((u32)src, (u32)src + len);
                return 0;
            }
        }
    else {
        mem_entry* nxt = &entries[idx + 1];

        if ((nxt->base - e->base) >= size) {
            e->limit = e->base + size;
            return ptr;
        } else {
            u8* src = (u8*)(e->base);
            u32 len = e->limit - (u32)src;

            kmm_removeEntry((u32)src);

            void* ret = kmalloc(size);

            if (ret != 0)
                return memcpy(ret, src, size);
            else {
                kmm_addEntry((u32)src, (u32)src + len);
                return 0;
            }
        }
    }
}

void* kmalloc_pa(u32 size) {
    u32 last_limit = KHEAP_BASE;

    for (int i = 0; i < entry_count; i++) {
        mem_entry* e = &entries[i];

        u32 aligned_limit = round_up(last_limit, 4096);

        if (aligned_limit < e->base && (e->base - aligned_limit) >= size)
            if (!kmm_addEntry(aligned_limit, aligned_limit + size)) return (void*)0;
            else return (void*)aligned_limit;

        last_limit = e->limit;
    }
    
    u32 aligned_limit = round_up(last_limit, 4096);

    if (aligned_limit < KHEAP_LIMIT && (KHEAP_LIMIT - aligned_limit) >= size)
        if (!kmm_addEntry(aligned_limit, aligned_limit + size)) return (void*)0;
        else return (void*)aligned_limit;
    else {
        dbg_write("kmalloc_pa: out of memory\n");
        return (void*)0;
    }
}

int __stopit = 0;

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
    else {
        dbg_write("kmalloc: out of memory\n");
        return (void*)0;
    }
}

void kfree(void* block) {
    kmm_removeEntry((u32) block);
}