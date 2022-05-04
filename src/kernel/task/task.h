#pragma once

#include "../utils.h"
#include "../vmem.h"
#include "../int/isr.h"
#include "../fs/apofs.h"
#include "../kmm.h"
#include "fd.h"

#define STDIN  0
#define STDOUT 1
#define STDERR 2

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2

typedef struct task {
    u32    pid;
    vmem*  mem;

    isr_regs* regs;

    u8     running;

    ARRAY  blocks;

    u32    entry;
    u32    stack;
    u32    heap;

    u32    heap_limit;

    u32    esp;
    u32    ebp;

    PARRAY fds;

    fd*    stdin;
    fd*    stdout;
    fd*    stderr;
} task;

extern u32 task_previous_esp;
extern u32 task_previous_ebp;

extern u32 task_next_esp;
extern u32 task_next_ebp;

void task_init();

task* task_create(const char* path);

task* task_get(u32 pid);

void task_close(task* t, int code);

task* task_getCurrent();

fd* task_createStdStream(task* task, int type);

int task_attach(task* task, fd* fd);

fd* task_open(const char* path, int flags);

u32 task_read(fd* fd, char* buffer, u32 size);

u32 task_write(fd* fd, char* buffer, u32 size);

u32 task_seek(fd* fd, int offset, int whence);

u32 task_tell(fd* fd);

void* task_realloc(void* ptr, u32 size);

void* task_malloc(u32 size);

void task_free(void* block);

void task_switch();

void task_exception(int type, int err_code);

// END OF KERNEL CODE EXECUTION
void task_resume(task* t);