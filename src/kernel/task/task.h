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

extern u32 task_previous_esp;
extern u32 task_previous_ebp;

extern u32 task_next_esp;
extern u32 task_next_ebp;

void task_init();

task* task_open(apo_fs* fs, const char* path);

void task_switch();

void task_exception(int type, int err_code);

// END OF KERNEL CODE EXECUTION
void task_resume(task* t);