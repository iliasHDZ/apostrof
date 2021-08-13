#include "task.h"
#include "../kmm.h"
#include "../vga.h"

u32 task_last_id = 0;

task* task_open(apo_fs* fs, const char* path) {
    if (fs == 0 || path == 0) return 0;

    u32 file_id = apofs_getFile(fs, path);
    if (file_id == 0) return 0;

    u32 file_size = apofs_getFileSize(fs, file_id);
    if (file_size == 0) return 0;

    task* ret = (task*)kmalloc(sizeof(task));
    if (ret == 0) return 0;

    vmem* task_memory = vmem_createTaskMemory(file_size, 4096 * 16);
    
    if (task_memory == 0) {
        kfree(ret);
        return 0;
    }

    vmem_switchMemory(task_memory);

    if (apofs_read(fs, file_id, (u8*)(VMEM_KERNEL_PAGES * 4096), 0, file_size) < file_size) {
        vmem_switchMemory(kernel_memory);
        vmem_freeMemory(task_memory);
        kfree(ret);
        return 0;
    }

    ret->pid     = task_last_id++;
    ret->mem     = task_memory;

    ret->regs    = 0;

    ret->running = 0;

    ret->entry   = VMEM_KERNEL_PAGES * 4096;
    ret->stack   = VMEM_TASK_STACK;

    ret->esp     = 0;
    ret->ebp     = 0;
}

void task_resume(task* t) {
    vmem_switchMemory(t->mem);

    if (!t->running) {
        t->running = 1;

        asm("mov %%eax, %%esp;"
            "mov %%esp, %%ebp;"
            "jmp %%ebx;" : : "a"(t->stack), "b"(t->entry));
    } else {
        // RETURN FROM INTERRUPT
    }
}