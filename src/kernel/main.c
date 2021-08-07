#include "vga.h"

#include "int/isr.h"
#include "int/idt.h"

#include "drv/timer.h"
#include "drv/keyboard.h"
#include "drv/keys.h"
#include "drv/pci.h"
#include "drv/ide.h"
#include "drv/storage.h"

#include "app/shell.h"

#include "fs/apofs.h"

#include "kmm.h"

#include "vmem.h"

#include "error.h"

#include "io.h"

#define VGA_CRTC_INDEX		0x3D4
#define VGA_CRTC_DATA		0x3D5

void error_handler(const char* err) {
    vga_setColor(VGA_RED, VGA_BLACK);
    vga_write("\nERROR: ");
    vga_write(err);
    vga_writeChar('\n');

    asm("cli");
    while (1) {};
}

extern u32 last_kernel_cookie;

void kernel_main() {
    kmm_init();

    vga_init();
    error_register(error_handler);

    vga_setColor(VGA_WHITE, VGA_BLACK);
    vga_clear();

    vga_print("Initializing kernel...");

    isr_init();
    idt_init();

    vga_print("PIC and ISR initialized");

    timer_init();

    vga_print("Timer initialized");

    keyboard_init();

    vga_print("Keyboard initialized\n");

    pci_init();

    storage_init();
    ide_init();

    apofs_init();

    vga_setColor(VGA_LIGHT_GREEN, VGA_BLACK);

    vga_print("Welcome to Apostrof'");

    vmem_init();
    shell_init();
}