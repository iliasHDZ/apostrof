#include "task.h"
#include "../kmm.h"
#include "../vga.h"

// From assembly at /src/kernel/int/isr_a.asm
extern void interrupt_return();

u32 task_last_id = 0;

u32 task_previous_esp = 0;
u32 task_previous_ebp = 0;

u32 task_next_esp  = 0;
u32 task_next_ebp  = 0;

task*  task_current = 0;

task** task_list       = 0;

u32    task_list_count = 0;
u32    task_list_max   = 0;

u32    task_list_current = 0;

void task_list_push(task* t) {
    if (task_list_count >= task_list_max) {
        task_list_max += 10;
        task_list = krealloc(task_list, task_list_max * sizeof(task**));
    }

    task_list[task_list_count++] = t;
}

void task_list_remove(u32 pid) {
    for (int i = 0; i < task_list_count; i++)
        if (task_list[i]->pid == pid) {
            task_list_count--;
            for (; i < task_list_count; i++)
                task_list[i] = task_list[i + 1];

            return;
        }
}

void task_init() {
    task_list_max   = 10;
    task_list_count = 0;

    task_list = kmalloc(task_list_max * sizeof(task**));
}

task* task_open(apo_fs* fs, const char* path) {
    if (fs == 0 || path == 0) return 0;

    u32 file_id = apofs_getFile(fs, path);
    if (file_id == 0) return 0;

    u32 file_size = apofs_getFileSize(fs, file_id);
    if (file_size == 0) return 0;

    task* ret = (task*)kmalloc(sizeof(task));
    if (ret == 0) return 0;

    vmem* task_memory = vmem_createTaskMemory(file_size, 4096 * 1);
    
    if (task_memory == 0) {
        kfree(ret);
        return 0;
    }

    vmem_switchMemory(task_memory);

    if (apofs_read(fs, file_id, VMEM_TASK_CODE, 0, file_size) < file_size) {
        vmem_switchMemory(kernel_memory);
        vmem_freeMemory(task_memory);
        kfree(ret);
        return 0;
    }

    ret->pid     = task_last_id++;
    ret->mem     = task_memory;

    ret->regs    = 0;

    ret->running = 0;

    ret->entry   = VMEM_TASK_CODE;
    ret->stack   = VMEM_TASK_STACK;

    ret->esp     = 0;
    ret->ebp     = 0;

    task_list_push(ret);
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

        interrupt_return();
    }
}

void task_switch() {
    if (task_current == 0) {
        task_next_esp = task_previous_esp;
        task_next_ebp = task_previous_ebp;
        interrupt_return();
    }

    task_current->esp = task_previous_esp;
    task_current->ebp = task_previous_ebp;

    task_list_current++;
    if (task_list_current >= task_list_count) task_list_current = 0;

    task_resume(task_list[task_list_current]);
}

void task_exception(int type, int err_code) {
    // oh no
}
