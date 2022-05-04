#include "vga.h"

#include "int/isr.h"
#include "int/idt.h"

#include "drv/timer.h"
#include "drv/keyboard.h"
#include "drv/keys.h"
#include "drv/pci.h"
#include "drv/ide.h"
#include "drv/drive.h"
#include "shell/shell.h"

#include "fs/apofs.h"

#include "task/task.h"
#include "task/syscall.h"

#include "kmm.h"
#include "vmem.h"
#include "error.h"
#include "io.h"
#include "dbg.h"

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

    dbg_write("Initializing kernel...\n");

    dbg_write("Initializing memory manager... ");
    kmm_init();
    dbg_write("OK\n");

    vga_init();
    error_register(error_handler);

    vga_setColor(VGA_WHITE, VGA_BLACK);
    vga_clear();

    //vga_print("Initializing kernel...");

    dbg_write("Initializing interrupts... ");
    isr_init();
    idt_init();
    dbg_write("OK\n");

    dbg_write("Initializing basic devices... ");
    timer_init();
    keyboard_init();
    dbg_write("OK\n");

    dbg_write("Initializing PCI... ");
    pci_init();
    dbg_write("OK\n");
    drive_init();
    ide_init();
    //apofs_init();

    dbg_write("Initializing virtual memory... ");
    vmem_init();
    dbg_write("OK\n");

    dbg_write("Initializing multitasking... ");
    syscall_init();
    task_init();
    dbg_write("OK\n");

    vga_writeText("Welcome to ");

    vga_setColor(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_writeText("Apostrof'\n\n");

    shell_init();

    //vga_setCursor(0, 10);

    //task* t2 = task_create(current_root, "/test_app");

    //task_resume(t2);

    asm("sti");

    for (;;);
}