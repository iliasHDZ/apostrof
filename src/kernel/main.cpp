extern "C" {
#include "vga.h"

#include "int/isr.h"
#include "int/idt.h"

#include "drv/timer.h"
#include "drv/keyboard.h"
#include "drv/keys.h"
#include "drv/pci.h"
#include "drv/ide.h"
#include "drv/drive.h"

#include "fs/apofs.h"

#include "task/task.h"
#include "task/syscall.h"

#include "kmm.h"
#include "vmem.h"
#include "error.h"
#include "io.h"
#include "dbg.h"
}

#include "shell/shell.hpp"
#include "fs/filesystem.hpp"

using namespace apo;

extern "C" void error_handler(const char* err) {
    vga_setColor(VGA_RED, VGA_BLACK);
    vga_writeText("\nERROR: ");
    vga_writeText(err);
    vga_writeChar('\n');
    dbg_write("\nERROR: ");
    dbg_write(err);
    dbg_write("\n");

    asm("cli");
    while (1) {};
}

extern "C" void __cxa_pure_virtual() {
    error("pure virtual function called");
}

extern u32 last_kernel_cookie;

extern "C" u32 kernel_stack_base = 0;

extern "C" void kernel_main(u32 stack) {
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

    dbg_write("Initializing filesystem...\n");

    Filesystem* fs = Filesystem::mount(drive_getDevice(0));
    if (fs == nullptr)
        return error("Failed to mount boot drive");

    Filesystem::setCurrent(fs);

    dbg_write("Initializing virtual memory... ");
    vmem_init();
    dbg_write("OK\n");

    dbg_write("Initializing multitasking... ");
    syscall_init();
    task_init();
    dbg_write("OK\n");

    /*vga_writeText("Welcome to ");

    vga_setColor(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_writeText("Apostrof'\n\n");

    shell_init();*/

    task* t = task_create("/usr/bin/terminal");

    if (t != 0)
        task_resume(t);

    asm("sti");

    for (;;);
}