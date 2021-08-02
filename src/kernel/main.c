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

    storage_init();
    ide_init();

    apo_fs* fs = apofs_openDevice(storage_getDevice(0));

    if (fs != 0) {
        char file_name[60];

        if (apofs_getFileName(fs, 1, file_name, 59) == 0) {
            file_name[59] = 0;

            vga_print(file_name);

            u32 file = apofs_getFile(fs, "this IS A very COOL FIle/test_file");

            if (file == 0) {
                vga_write("Failed to find file ");
                vga_writeWord(apofs_lastError());
                vga_writeChar('\n');
            } else {
                vga_write("File found! file_id: ");
                vga_writeInteger(file);
                vga_writeChar('\n');
            }
        } else {
            vga_write("Failed to get filename ");
            vga_writeWord(apofs_lastError());
            vga_writeChar('\n');
        }
    } else {
        vga_write("Failed to open filesystem ");
        vga_writeWord(apofs_lastError());
        vga_writeChar('\n');
    }

    vga_setColor(VGA_LIGHT_GREEN, VGA_BLACK);

    vga_print("Welcome to Apostrof'");

    shell_init(input_buffer, SHELL_INPUT_BUFFER_SIZE);

    asm("sti");
    while (1) asm("hlt");
}