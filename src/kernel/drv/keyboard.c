#include "keyboard.h"

#include "../int/isr.h"
#include "../io.h"
#include "../error.h"
#include "../vga.h"

#include "kb_layout/nl_be_qemu.h"

u8 escape_e0 = 0;

KEY_HANDLER keydown_handler;
KEY_HANDLER keyup_handler;

CHAR_HANDLER char_handler;

void send_keydown(u16 scan) {
    nl_be_qemu_keydown(scan);
    if (keydown_handler != 0) keydown_handler(scan);
}

void send_keyup(u16 scan) {
    nl_be_qemu_keyup(scan);
    if (keyup_handler != 0) keyup_handler(scan);
}

void handle_scancode(u8 scancode) {
    if (scancode == 0x00 || scancode == 0xff) {
        error("Keyboard error");
        return;
    }

    u8 up  = scancode >> 7;
    u8 key = scancode & 0x7f;
    
    if (!escape_e0) {
        if (scancode == 0xe0) {
            escape_e0 = 1;
            return;
        }

        if (up) send_keyup(key);
        else send_keydown(key);
    } else {
        if (up) send_keyup(key | 0xe000);
        else send_keydown(key | 0xe000);
        escape_e0 = 0;
    }
}

void keyboard_keydown(KEY_HANDLER keydown) {
    keydown_handler = keydown;
}

void keyboard_keyup(KEY_HANDLER keyup) {
    keyup_handler = keyup;
}

void keyboard_char(CHAR_HANDLER handler) {
    char_handler = handler;
}

void send_char(char c) {
    if (char_handler != 0) char_handler(c);
}

static void keyboard_callback() {
    u8 scancode = inb(0x60);
    
    handle_scancode(scancode);
}

void keyboard_init() {
    isr_register(IRQ1, keyboard_callback);
}