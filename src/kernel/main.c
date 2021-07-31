#include "vga.h"

#include "int/isr.h"
#include "int/idt.h"

#include "drv/timer.h"
#include "drv/keyboard.h"
#include "drv/keys.h"
#include "drv/pci.h"
#include "drv/ide.h"

#include "app/shell.h"

#include "kmm.h"

#include "gdt.h"
#include "error.h"

#define SHELL_INPUT_BUFFER_SIZE 128

void error_handler(const char* err) {
    vga_setColor(VGA_RED, VGA_BLACK);
    vga_write("\nERROR: ");
    vga_write(err);
    vga_writeChar('\n');

    asm("cli");
    while (1) {};
}

void kernel_main() {
    char input_buffer[SHELL_INPUT_BUFFER_SIZE];

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
    ide_init();

    vga_setColor(VGA_LIGHT_GREEN, VGA_BLACK);

    vga_print("Welcome to tes-os by IliasHDZ!");

    shell_init(input_buffer, SHELL_INPUT_BUFFER_SIZE);

    asm("sti");
    while (1) asm("hlt");
}