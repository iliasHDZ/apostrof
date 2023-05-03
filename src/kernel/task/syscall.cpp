extern "C" {
#include "syscall.h"
#include "../int/isr.h"
#include "../vga.h"
}

#include "task.hpp"

using namespace apo;

u32 syscall_exec(u32 a, u32 b, u32 c, u32 d) {
    Task* task;
    fd* fdn;

    switch (a)
    {
    case 0x01: // exit
        Task::current()->close(b); break;
    
    case 0x10: // read
        return Task::read((fd*)b, (char*)c, d);
    case 0x11: // write
        return Task::write((fd*)b, (char*)c, d);
    case 0x12: // seek
        return Task::seek((fd*)b, (int)c, (int)d);
    case 0x13: // tell
        return Task::tell((fd*)b);

    case 0x30: // malloc
        return (u32)Task::malloc(b);
    case 0x31: // free
        Task::free((void*)b); break;
    case 0x32: // realloc
        return (u32)Task::realloc((void*)b, c);
    
    case 0x40: // process_create
        return Task::create((const char*)b)->pid;
    case 0x41: // process_getStdStream
        task = Task::get(b);
        if (task == 0) break;

        fdn = task->createStdStream(c);
        if (fdn == 0) break;

        Task::current()->attach(fdn);
        return (u32)fdn;
    case 0x42: // process_current
        return Task::current()->pid;
    case 0x43: // process_createWithStdout
        task = Task::create((const char*)b);
        if (task == 0) break;

        fdn = task->createStdStream(STDOUT);
        if (fdn == 0) break;

        Task::current()->attach(fdn);
        *(fd**)c = fdn;
        return task->pid;

    case 0x60: // open
        return (u32)Task::open((const char*)b, (int)c);
    
    case 0x80: // video_get
        return vga_get((int)b);
    case 0x81: // video_set
        return vga_set((int)b, (int)c);
    case 0x120: // dbg_writeChar
        dbg_writeChar((char)b);
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