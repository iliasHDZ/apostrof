#include "syscall.h"
#include "../int/isr.h"

#include "../vga.h"
#include "task.h"

void syscall_run(u32 eax, isr_regs* regs) {
    switch (eax)
    {
    case 0x10: // fread
        regs->eax = task_read((fd*)(regs->ebx), (char*)(regs->ecx), regs->edx);
        break;
    case 0x11: // fwrite
        regs->eax = task_write((fd*)(regs->ebx), (char*)(regs->ecx), regs->edx);
        break;
    case 0x12: // fseek
        regs->eax = task_seek((fd*)(regs->ebx), (int)(regs->ecx), (int)(regs->edx));
        break;
    case 0x13: // ftell
        regs->eax = task_tell((fd*)(regs->ebx));
        break;
    case 0x30: // malloc
        regs->eax = task_malloc(regs->ebx);
        //vga_writeText(" MALLOC(");
        //vga_writeInteger(regs->ebx);
        //vga_writeText(") ");
        //vga_writeDWord((u32)regs->eax);
        break;
    case 0x31: // free
        task_free((void*)(regs->ebx));
        //vga_writeText(" FREE ");
        //vga_writeDWord((u32)regs->ebx);
        break;
    case 0x32: // realloc
        regs->eax = task_realloc((void*)(regs->ebx), regs->ecx);
        break;

    case 0x80: // vga_open
        regs->eax = (u32)vga_open();
        break;
    case 0x81: // vga_set
        regs->eax = vga_set((int)(regs->ebx), (int)(regs->ecx));
        break;
    case 0x82: // vga_get
        regs->eax = vga_get((int)(regs->ebx));
        break;
    
    default:
        break;
    }
}

void syscall_handler(isr_regs* regs) {
    syscall_run(regs->eax, regs);
}

void syscall_init() {
    isr_register(0x1C, syscall_handler);
}