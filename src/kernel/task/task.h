#pragma once

#include "../utils.h"
#include "../vmem.h"
#include "../int/isr.h"
#include "../fs/apofs.h"

typedef struct task {
    u32   pid;
    vmem* mem;

    isr_regs* regs;

    u8    running;

    u32   entry;
    u32   stack;

    u32   esp;
    u32   ebp;
} task;

task* task_open(apo_fs* fs, const char* path);

// END OF KERNEL CODE EXECUTION
void task_resume(task* t);