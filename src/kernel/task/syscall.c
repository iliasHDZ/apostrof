#include "syscall.h"
#include "../int/isr.h"

#include "../vga.h"

void syscall_run(u32 eax, isr_regs* regs) {
    switch (eax)
    {
    case 0x10:
        vga_print((const char*)(regs->ebx));
        break;
    case 0x80: // vga_seek
        regs->eax = vga_seek((int)(regs->ebx), (int)(regs->ecx));
        break;
    case 0x81: // vga_tell
        regs->eax = vga_tell();
        break;
    case 0x82: // vga_read
        regs->eax = vga_read((char*)(regs->ebx), (int)(regs->ecx));
        break;
    case 0x83: // vga_write
        regs->eax = vga_write((char*)(regs->ebx), (int)(regs->ecx));
        break;
    case 0x84: // vga_set
        regs->eax = vga_set((int)(regs->ebx), (int)(regs->ecx));
        break;
    case 0x85: // vga_get
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