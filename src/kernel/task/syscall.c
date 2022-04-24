#include "syscall.h"
#include "../int/isr.h"

#include "../vga.h"
#include "task.h"

u32 syscall_exec(u32 a, u32 b, u32 c, u32 d) {
    task* tsk;
    fd* fdn;

    switch (a)
    {
    case 0x01: // exit
        task_close(task_getCurrent(), b); break;
    
    case 0x10: // read
        return task_read((fd*)b, (char*)c, d);
    case 0x11: // write
        return task_write((fd*)b, (char*)c, d);
    case 0x12: // seek
        return task_seek((fd*)b, (int)c, (int)d);
    case 0x13: // tell
        return task_tell((fd*)b);

    case 0x30: // malloc
        return task_malloc(b);
    case 0x31: // free
        task_free((void*)b); break;
    case 0x32: // realloc
        return task_realloc((void*)b, c);
    
    case 0x40: // process_create
        return task_create(current_root, (const char*)b)->pid;
    case 0x41: // process_getStdStream
        tsk = task_get(b);
        if (tsk == 0) break;

        fdn = task_createStdStream(tsk, c);
        if (fdn == 0) break;

        task_attach(task_getCurrent(), fdn);
        return fdn;
    case 0x42: // process_current
        return task_getCurrent()->pid;
    case 0x43: // process_createWithStdout
        tsk = task_create(current_root, (const char*)b);
        if (tsk == 0) break;

        fdn = task_createStdStream(tsk, STDOUT);
        if (fdn == 0) break;

        task_attach(task_getCurrent(), fdn);
        *(u32*)c = fdn;
        return tsk->pid;

    case 0x60: // open
        return task_open(current_root, (const char*)b, (int)c);
    
    case 0x80: // video_get
        return vga_get((int)b);
    case 0x81: // video_set
        return vga_set((int)b, (int)c);
    default:
        break;
    }

    return 0;
}

void syscall_run(u32 eax, isr_regs* regs) {
    regs->eax = syscall_exec(eax, regs->ebx, regs->ecx, regs->edx);
}

void syscall_handler(isr_regs* regs) {
    syscall_run(regs->eax, regs);
}

void syscall_init() {
    isr_register(0x1C, syscall_handler);
}