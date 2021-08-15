#include "task.h"
#include "../vga.h"

// From assembly at /src/kernel/int/isr_a.asm
extern void interrupt_return(u32 esp, u32 ebp);

u32 task_last_id = 0;

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

task* task_open(apo_fs* fs, const char* path) {
    if (fs == 0 || path == 0) return 0;

    u32 file_id = apofs_getFile(fs, path);
    if (file_id == 0) return 0;

    u32 file_size = apofs_getFileSize(fs, file_id);
    if (file_size == 0) return 0;

    task* ret = (task*)kmalloc(sizeof(task));
    if (ret == 0) return 0;

    if (parray(&(ret->fds), 10) != 0) return 0;

    vmem* task_memory = vmem_createTaskMemory(file_size, 4096 * 16);
    
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

    parray_push(&tasklist, ret);
    return ret;
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

u32 task_read(fd* fd, char* buffer, u32 size) {
    if (!task_fd_present(task_current, fd)) return 0;

    if (fd->type == FD_RO_FBUFFER || fd->type == FD_RW_FBUFFER)
        return fd_read(fd, buffer, size);
}

u32 task_write(fd* fd, char* buffer, u32 size) {
    if (!task_fd_present(task_current, fd)) return 0;

    if (fd->type == FD_RW_FBUFFER)
        return fd_write(fd, buffer, size);
}

u32 task_tell(fd* fd) {
    if (!task_fd_present(task_current, fd)) return 0;

    return fd_tell(fd);
}

u32 task_seek(fd* fd, int offset, int whence) {
    if (!task_fd_present(task_current, fd)) return 0;

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

    task_resume(parray_get(&tasklist, task_list_current));
}

void task_exception(int type, int err_code) {
    // oh no
}
