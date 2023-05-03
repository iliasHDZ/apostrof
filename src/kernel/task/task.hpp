#pragma once

extern "C" {
#include "../utils.h"
#include "../vmem.h"
#include "../int/isr.h"
#include "fd.h"
}
#include "../utils.hpp"

#define STDIN  0
#define STDOUT 1
#define STDERR 2

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2

extern u32 task_previous_esp;
extern u32 task_previous_ebp;

extern u32 task_next_esp;
extern u32 task_next_ebp;

namespace apo {

typedef struct {
    u32 base;
    u32 limit;
} task_heap_block;

class Task {
private:
    Task();

public:
    ~Task();

    static void init();

    static Task* create(const char* path);

    static Task* get(u32 pid);

    static Task* current();

    static fd* open(const char* path, int flags);

    static u32 read(fd* fd, char* buffer, u32 size);

    static u32 write(fd* fd, char* buffer, u32 size);

    static u32 seek(fd* fd, int offset, int whence);

    static u32 tell(fd* fd);

    static void* realloc(void* ptr, u32 size);

    static void* malloc(u32 size);

    static void free(void* block);

    static void switchTask();

    static void exception(int type, int err_code);

public:
    fd* createStdStream(int type);

    fd* getfd(u32 fdn);

    int attach(fd* fd);

    void close(int code);

    // END OF KERNEL CODE EXECUTION
    void resume();

public:
    u32   pid;
    vmem* memory;

    isr_regs* registers;

    u8 running;

    u32 entry;
    u32 stack;
    u32 heap;

    u32 heap_brk;

    Array<fd*> fds;
    Array<task_heap_block> blocks;

    fd* stdout;
    fd* stdin;
    fd* stderr;

    u32 esp;
    u32 ebp;
};

}