#include "task.hpp"
#include "elf32.hpp"

extern "C" {
#include "../vga.h"
}

using namespace apo;

u32 task_previous_esp = 0;
u32 task_previous_ebp = 0;

u32 task_next_esp = 0;
u32 task_next_ebp = 0;

// From assembly at /src/kernel/int/isr_a.asm
extern "C" void interrupt_return(u32 esp, u32 ebp);

u32 task_last_id = 1;

Task::Task() {}

Task::~Task() {
    // TODO: Delete all properties
    fds.~Array();
    blocks.~Array();

    vmem_freeMemory(memory);
}
    
Array<Task*>* tasklist;
Task* currentTask;

void Task::init() {
    tasklist = new Array<Task*>(10);
}

Task* Task::create(const char* path) {
    auto task = new Task();
    if (task == nullptr) return nullptr;

    char* error = "";

    vmem* last_mem = vmem_current();

    if (!elf32_load(path, task, 4096 * 16, &error)) {
        vga_writeText(error);
        return 0;
    }

    task->pid       = task_last_id++;
    task->registers = nullptr;
    task->running   = 0;
    task->esp       = 0;
    task->ebp       = 0;

    task->stdin  = fd_createInStream();
    task->stdout = void_stream;
    task->stderr = void_stream;

    vmem_switchMemory(last_mem);

    tasklist->push(task);
    return task;
}

Task* Task::get(u32 pid) {
    for (auto task : *tasklist)
        if (task->pid == pid) return task;

    return nullptr;
}

Task* Task::current() {
    return currentTask;
}

task_heap_block* taskmm_getEntryFromIndex(u32 index);

u8 taskmm_addEntry(u32 base, u32 limit) {
    int i = 0;
    for (; i < currentTask->blocks.size(); i++) {
        task_heap_block* block = taskmm_getEntryFromIndex(i);
        if (block->base > base) break;
    }

    task_heap_block block = (task_heap_block){base, limit};

    currentTask->blocks.insert(block, i);
    return 1;
}

int taskmm_getEntryIndex(u32 base) {
    int e = 0;

    task_heap_block block = currentTask->blocks[e];
    
    while (e < currentTask->blocks.size()) {
        if (block.base == base) return e;
        if (++e < currentTask->blocks.size())
            block = currentTask->blocks[e];
    }

    return -1;
}

u8 taskmm_removeEntry(u32 base) {
    int idx = taskmm_getEntryIndex(base);
    if (idx == -1) return 0;

    currentTask->blocks.remove(idx);
    return 1;
}

task_heap_block* taskmm_getEntryFromIndex(u32 index) {
    return &currentTask->blocks[index];
}

task_heap_block* taskmm_getEntry(u32 base) {
    int idx = taskmm_getEntryIndex(base);
    if (idx == -1) return 0;

    return taskmm_getEntryFromIndex(idx);
}

int taskmm_allocHeapPages(u32 count) {
    for (int i = 0; i < count; i++) {
        vmem_allocPage(currentTask->memory, currentTask->heap_brk, VMEM_PRESENT | VMEM_WRITABLE | VMEM_USER);
        currentTask->heap_brk += 4096;
    }
}

int taskmm_freeHeapPages(u32 count) {
    for (int i = 0; i < count; i++) {
        currentTask->heap_brk -= 4096;
        vmem_freePage(currentTask->memory, currentTask->heap_brk);
    }
}

void taskmm_collect() {
    u32 max_limit = currentTask->heap;

    for (int i = 0; i < currentTask->blocks.size(); i++) {
        task_heap_block* block = taskmm_getEntryFromIndex(i);
        if (block->limit > max_limit) max_limit = block->limit;
    }

    int limit_page = (max_limit - currentTask->heap - 1) / 4096;
    int heap_limit_page = (currentTask->heap_brk - currentTask->heap - 1) / 4096;

    if (limit_page < heap_limit_page)
        taskmm_freeHeapPages(heap_limit_page - limit_page);
}

fd* Task::open(const char* path, int flags) {
    Task* task = currentTask;
    if (task == 0) return 0;

    if (cmpstr(path, "/dev/fb"))
        return vga_open();

    return 0;
}

u32 Task::read(fd* fd, char* buffer, u32 size) {
    fd = currentTask->getfd((u32)fd);
    if (fd == 0) return 0;

    if (fd->type == FD_RO_FBUFFER || fd->type == FD_RW_FBUFFER || fd->type == FD_IN_STREAM)
        return fd_read(fd, buffer, size);
}

u32 Task::write(fd* fd, char* buffer, u32 size) {
    fd = currentTask->getfd((u32)fd);

    if (fd == 0) return 0;

    if (fd->type == FD_RW_FBUFFER || fd->type == FD_OUT_STREAM || fd->type == FD_VOID)
        return fd_write(fd, buffer, size);
}

u32 Task::tell(fd* fd) {
    fd = currentTask->getfd((u32)fd);
    if (fd == 0) return 0;

    return fd_tell(fd);
}

u32 Task::seek(fd* fd, int offset, int whence) {
    fd = currentTask->getfd((u32)fd);
    if (fd == 0) return 0;

    return fd_seek(fd, offset, whence);
}

void* Task::realloc(void* ptr, u32 size) {
    int idx = taskmm_getEntryIndex((u32)ptr);
    if (idx == -1) return Task::malloc(size);

    task_heap_block* e = taskmm_getEntryFromIndex(idx);

    if (idx + 1 >= currentTask->blocks.size())
        if ((currentTask->heap_brk - e->base) >= size) {
            e->limit = e->base + size;
            return ptr;
        } else {
            u32 excess = e->base + size - currentTask->heap_brk;
            u32 prevsz = currentTask->heap_brk - e->base;

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

            void* ret = Task::malloc(size);

            if (ret != 0)
                return memcpy(ret, src, size);
            else {
                taskmm_addEntry((u32)src, (u32)src + len);
                return 0;
            }
        }
    }
}

void* Task::malloc(u32 size) {
    u32 last_limit = currentTask->heap;

    for (int i = 0; i < currentTask->blocks.size(); i++) {
        task_heap_block* e = taskmm_getEntryFromIndex(i);

        if ((e->base - last_limit) >= size) {
            if (!taskmm_addEntry(last_limit, last_limit + size)) return (void*)0;
            else return (void*)last_limit;
        }

        last_limit = e->limit;
    }

    if ((currentTask->heap_brk - last_limit) >= size)
        if (!taskmm_addEntry(last_limit, last_limit + size)) return (void*)0;
        else return (void*)last_limit;
    else {
        u32 excess = last_limit + size - currentTask->heap_brk;

        if (excess > 0) {
            taskmm_allocHeapPages(excess / 4096 + (excess % 4096 > 0));
            if (!taskmm_addEntry(last_limit, last_limit + size)) return (void*)0;
            else return (void*)last_limit;
        }
    }
}

void Task::free(void* block) {
    taskmm_removeEntry((u32) block);
    taskmm_collect();
}

void Task::switchTask() {
    if (currentTask == 0) {
        task_next_esp = task_previous_esp;
        task_next_ebp = task_previous_ebp;
        interrupt_return(task_next_esp, task_next_ebp);
    }

    currentTask->esp = task_previous_esp;
    currentTask->ebp = task_previous_ebp;

    u32 idx = tasklist->indexOf(currentTask);

    idx++;
    if (idx >= tasklist->size()) idx = 0;

    (*tasklist)[idx]->resume();
}

void Task::exception(int type, int err_code) {
    vga_setCursor(0, 2);
    vga_writeText("I'm pretty sure a task exception happened.\n");
}

fd* Task::createStdStream(int type) {
    fd* ret;

    switch (type) {
    case STDIN:
        return fd_createOutStream(stdin);
    case STDOUT:
        ret = fd_createInStream();
        stdout = fd_createOutStream(ret);
        return ret;
    case STDERR:
        ret = fd_createInStream();
        stderr = fd_createOutStream(ret);
        return ret;
    }
}

fd* Task::getfd(u32 fdn) {
    switch ((u32)fdn) {
    case STDIN:  return stdin;
    case STDOUT: return stdout == 0 ? void_stream : stdout;
    case STDERR: return stderr == 0 ? void_stream : stderr;
    }

    for (int i = 0; i < fds.size(); i++) {
        fd* f = fds[i];
        if (f == (fd*)fdn)
            return f;
    }

    return 0;
}

int Task::attach(fd* f) {
    fds.push(f);
}

void Task::close(int code) {
    for (int i = 0; i < tasklist->size(); i++) {
        if ((*tasklist)[i] == this) {
            tasklist->remove(i);
            break;
        }
    }

    delete this;
}

void Task::resume() {
    vmem_switchMemory(memory);
    currentTask = this;

    if (!running) {
        running = 1;

        asm("sti\n"
            "mov %%eax, %%esp\n"
            "mov %%esp, %%ebp\n"
            "jmp %%ebx\n" : : "a"(stack), "b"(entry));
    } else {
        u32 task_next_esp = esp;
        u32 task_next_ebp = ebp;

        interrupt_return(task_next_esp, task_next_ebp);
    }
}