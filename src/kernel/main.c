#include "vga.h"

#include "int/isr.h"
#include "int/idt.h"

#include "drv/timer.h"
#include "drv/keyboard.h"
#include "drv/keys.h"
#include "drv/pci.h"
#include "drv/ide.h"
#include "drv/storage.h"

#include "fs/apofs.h"

#include "task/task.h"
#include "task/syscall.h"

#include "kmm.h"
#include "vmem.h"
#include "error.h"
#include "io.h"

#define VGA_CRTC_INDEX		0x3D4
#define VGA_CRTC_DATA		0x3D5

void error_handler(const char* err) {
    /*vga_setColor(VGA_RED, VGA_BLACK);
    vga_write("\nERROR: ");
    vga_write(err);
    vga_writeChar('\n');*/

    asm("cli");
    while (1) {};
}

extern u32 last_kernel_cookie;

u32 kernel_stack_base;

vmem* taska;

void kernel_main(u32 stack) {
    kernel_stack_base = stack;

    kmm_init();

    vga_init();
    error_register(error_handler);

    vga_setColor(VGA_WHITE, VGA_BLACK);
    vga_clear();

    //vga_print("Initializing kernel...");

    isr_init();
    idt_init();

    timer_init();
    keyboard_init();

    pci_init();
    storage_init();
    ide_init();
    apofs_init();

    vmem_init();

    syscall_init();
    task_init();

    /*vga_write("Welcome to ");

    vga_setColor(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("Apostrof'");*/

    task* t1 = task_open(current_root, "/test/task");
    task* t2 = task_open(current_root, "/test/app");

    task_resume(t1);

    //vga_writeDWord(kernel_stack_base);
    //shell_init();
}