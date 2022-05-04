extern "C" {
#include "task.h"
#include "../vga.h"
#include "../error.h"

#include "elf32.h"
}

// From assembly at /src/kernel/int/isr_a.asm
extern "C" void interrupt_return(u32 esp, u32 ebp);

extern "C" {
u32 task_last_id = 1;

u32 task_previous_esp = 0;
u32 task_previous_ebp = 0;

u32 task_next_esp  = 0;
u32 task_next_ebp  = 0;

task*  task_current = 0;

PARRAY tasklist;

u32    task_list_current = 0;

void task_init() {
    if (parray(&tasklist, 10) != 0)
        error("Cannot init 'task': kmalloc failed");
}

typedef struct {
    u32 base;
    u32 limit;
} task_heap_block;

task* task_create(const char* path) {
    task* ret = (task*)kmalloc(sizeof(task));
    if (ret == 0) return 0;

    char* error = "";

    vmem* last_mem = vmem_current();

    if (!elf32_load(path, ret, 4096 * 16, &error)) {
        vga_writeText(error);
        return 0;
    }

    if (parray(&(ret->fds), 10) != 0) return 0;
    ret->pid     = task_last_id++;
    ret->regs    = 0;
    ret->running = 0;
    ret->esp     = 0;
    ret->ebp     = 0;

    ret->stdin  = fd_createInStream();
    ret->stdout = void_stream;
    ret->stderr = void_stream;

    vmem_switchMemory(last_mem);

    array(&ret->blocks, sizeof(task_heap_block), 50);

    parray_push(&tasklist, ret);
    return ret;
}

task* task_get(u32 pid) {
    for (int i = 0; i < tasklist.count; i++) {
        task* t = (task*)parray_get(&tasklist, i);
        if (t->pid == pid) return t;
    }
    return 0;
}

void task_close(task* t, int code) {
    parray_remove(&tasklist, t);

    for (int i = 0; i < t->fds.count; i++)
        fd_close((fd*)parray_get(&(t->fds), i));
    
    parray_free(&(t->fds));

    vmem_freeMemory(t->mem);
    array_free(&(t->blocks));
    kfree(t);
}

task* task_getCurrent() {
    return task_current;
}

int task_attach(task* t, fd* fd) {
    parray_push(&(t->fds), fd);
}

u8 task_fd_present(task* t, fd* fd) {
    for (int i = 0; i < t->fds.count; i++)
        if (parray_get(&(t->fds), i) == fd) return 1;

    return 0;
}

void task_resume(task* t) {
    vmem_switchMemory(t->mem);
    task_current = t;

    if (!t->running) {
        t->running = 1;

        asm("sti;"
            "mov %%eax, %%esp;"
            "mov %%esp, %%ebp;"
            "jmp %%ebx;" : : "a"(t->stack), "b"(t->entry));
    } else {
        u32 task_next_esp = t->esp;
        u32 task_next_ebp = t->ebp;

        interrupt_return(task_next_esp, task_next_ebp);
    }
}

fd* task_getfd(task* t, u32 fdn) {
    switch ((u32)fdn) {
    case STDIN:  return t->stdin;
    case STDOUT: return t->stdout == 0 ? void_stream : t->stdout;
    case STDERR: return t->stderr == 0 ? void_stream : t->stderr;
    }

    for (int i = 0; i < t->fds.count; i++) {
        fd* f = (fd*)parray_get(&(t->fds), i);
        if (f == (fd*)fdn)
            return f;
    }

    return 0;
}

fd* task_createStdStream(task* task, int type) {
    fd* ret;

    switch (type) {
    case STDIN:
        return fd_createOutStream(task->stdin);
    case STDOUT:
        ret = fd_createInStream();
        task->stdout = fd_createOutStream(ret);
        return ret;
    case STDERR:
        ret = fd_createInStream();
        task->stderr = fd_createOutStream(ret);
        return ret;
    }
}

fd* task_open(const char* path, int flags) {
    task* t = task_getCurrent();
    if (t == 0) return 0;

    if (cmpstr(path, "/dev/fb"))
        return vga_open();

    return 0;
}

u32 task_read(fd* fd, char* buffer, u32 size) {
    fd = task_getfd(task_current, (u32)fd);
    if (fd == 0) return 0;

    if (fd->type == FD_RO_FBUFFER || fd->type == FD_RW_FBUFFER || fd->type == FD_IN_STREAM)
        return fd_read(fd, buffer, size);
}

u32 task_write(fd* fd, char* buffer, u32 size) {
    fd = task_getfd(task_current, (u32)fd);

    if (fd == 0) return 0;

    if (fd->type == FD_RW_FBUFFER || fd->type == FD_OUT_STREAM || fd->type == FD_VOID)
        return fd_write(fd, buffer, size);
}

u32 task_tell(fd* fd) {
    fd = task_getfd(task_current, (u32)fd);
    if (fd == 0) return 0;

    return fd_tell(fd);
}

u32 task_seek(fd* fd, int offset, int whence) {
    fd = task_getfd(task_current, (u32)fd);
    if (fd == 0) return 0;

    return fd_seek(fd, offset, whence);
}

void task_switch() {
    if (task_current == 0) {
        task_next_esp = task_previous_esp;
        task_next_ebp = task_previous_ebp;
        interrupt_return(task_next_esp, task_next_ebp);
    }

    task_current->esp = task_previous_esp;
    task_current->ebp = task_previous_ebp;

    task_list_current++;
    if (task_list_current >= tasklist.count) task_list_current = 0;

    task_resume((task*)parray_get(&tasklist, task_list_current));
}

task_heap_block* taskmm_getEntryFromIndex(u32 index);

u8 taskmm_addEntry(u32 base, u32 limit) {
    int i = 0;
    for (; i < task_current->blocks.count; i++) {
        task_heap_block* block = taskmm_getEntryFromIndex(i);
        if (block->base > base) break;
    }

    task_heap_block block = (task_heap_block){base, limit};

    array_insert(&(task_current->blocks), &block, i);
    return 1;
}

int taskmm_getEntryIndex(u32 base) {
    int e = 0;

    task_heap_block block;
    if (!array_get(&(task_current->blocks), e, &block))
        return -1;
    
    while (e < task_current->blocks.count) {
        if (block.base == base) return e;
        if (++e < task_current->blocks.count)
            array_get(&(task_current->blocks), e, &block);
    }

    return -1;
}

u8 taskmm_removeEntry(u32 base) {
    int idx = taskmm_getEntryIndex(base);
    if (idx == -1) return 0;

    array_remove(&(task_current->blocks), idx);
    return 1;
}

task_heap_block* taskmm_getEntryFromIndex(u32 index) {
    return (task_heap_block*)(task_current->blocks.buffer + index * task_current->blocks.size);
}

task_heap_block* taskmm_getEntry(u32 base) {
    int idx = taskmm_getEntryIndex(base);
    if (idx == -1) return 0;

    return (task_heap_block*)(task_current->blocks.buffer + idx * task_current->blocks.size);
}

int taskmm_allocHeapPages(u32 count) {
    for (int i = 0; i < count; i++) {
        vmem_allocPage(task_current->mem, task_current->heap_limit, VMEM_PRESENT | VMEM_WRITABLE | VMEM_USER);
        task_current->heap_limit += 4096;
    }
}

int taskmm_freeHeapPages(u32 count) {
    for (int i = 0; i < count; i++) {
        task_current->heap_limit -= 4096;
        vmem_freePage(task_current->mem, task_current->heap_limit);
    }
}

void taskmm_collect() {
    u32 max_limit = task_current->heap;

    for (int i = 0; i < task_current->blocks.count; i++) {
        task_heap_block* block = taskmm_getEntryFromIndex(i);
        if (block->limit > max_limit) max_limit = block->limit;
    }

    int limit_page = (max_limit - task_current->heap - 1) / 4096;
    int heap_limit_page = (task_current->heap_limit - task_current->heap - 1) / 4096;

    if (limit_page < heap_limit_page)
        taskmm_freeHeapPages(heap_limit_page - limit_page);
}

void* task_realloc(void* ptr, u32 size) {
    int idx = taskmm_getEntryIndex((u32)ptr);
    if (idx == -1) return task_malloc(size);

    task_heap_block* e = taskmm_getEntryFromIndex(idx);

    if (idx + 1 >= task_current->blocks.count)
        if ((task_current->heap_limit - e->base) >= size) {
            e->limit = e->base + size;
            return ptr;
        } else {
            u32 excess = e->base + size - task_current->heap_limit;
            u32 prevsz = task_current->heap_limit - e->base;

            if (excess > 0) {
                taskmm_allocHeapPages(excess / 4096 + (excess % 4096 > 0));
                e->limit = e->base + size;
            }

            if (prevsz > size)
                taskmm_collect();
        }
    else {
        task_heap_block* nxt = taskmm_getEntryFromIndex(idx + 1);

        if ((nxt->base - e->base) >= size) {
            e->limit = e->base + size;
            return ptr;
        } else {
            u8* src = (u8*)(e->base);
            u32 len = e->limit - (u32)src;

            taskmm_removeEntry((u32)src);

            void* ret = task_malloc(size);

            if (ret != 0)
                return memcpy(ret, src, size);
            else {
                taskmm_addEntry((u32)src, (u32)src + len);
                return 0;
            }
        }
    }
}

void* task_malloc(u32 size) {
    u32 last_limit = task_current->heap;

    for (int i = 0; i < task_current->blocks.count; i++) {
        task_heap_block* e = taskmm_getEntryFromIndex(i);

        if ((e->base - last_limit) >= size) {
            if (!taskmm_addEntry(last_limit, last_limit + size)) return (void*)0;
            else return (void*)last_limit;
        }

        last_limit = e->limit;
    }

    if ((task_current->heap_limit - last_limit) >= size)
        if (!taskmm_addEntry(last_limit, last_limit + size)) return (void*)0;
        else return (void*)last_limit;
    else {
        u32 excess = last_limit + size - task_current->heap_limit;

        if (excess > 0) {
            taskmm_allocHeapPages(excess / 4096 + (excess % 4096 > 0));
            if (!taskmm_addEntry(last_limit, last_limit + size)) return (void*)0;
            else return (void*)last_limit;
        }
    }
}

void task_free(void* block) {
    taskmm_removeEntry((u32) block);
    taskmm_collect();
}

void task_exception(int type, int err_code) {
    vga_setCursor(0, 2);
    vga_writeText("I'm pretty sure a task exception happened.\n");
}

}