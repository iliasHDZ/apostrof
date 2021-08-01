#include "vga.h"
#include "io.h"

char* vram  = (char*)0xb8000;
u8 color    = VGA_WHITE;

u8 cursor_x = 0;
u8 cursor_y = 0;

const char* hex = "0123456789ABCDEF";

void vga_setChar(char c, u8 x, u8 y) {
    if (x < 0 || x >= VGA_WIDTH ||
        y < 0 || y >= VGA_HEIGHT) return;

    int offset = (y * VGA_WIDTH + x) * 2;

    vram[offset]     = c;
    vram[offset + 1] = color;
}

void vga_scroll(int count) {
    for (int x = 0; x < VGA_WIDTH; x++)
        for (int y = 0; y < VGA_WIDTH; y++) {
            int get_y = y + count;

            if (get_y >= VGA_HEIGHT) vga_setChar(' ', x, y);
            else {
                int src = (get_y * VGA_WIDTH + x) * 2;
                int dst = (y * VGA_WIDTH + x) * 2;

                vram[dst]     = vram[src];
                vram[dst + 1] = vram[src + 1];
            }
        }

    vga_setCursor(cursor_x, cursor_y - count);
}

void vga_writeChar(char c) {
    if (c != '\n')
        vga_setChar(c, cursor_x, cursor_y);

    cursor_x++;
    if (cursor_x >= VGA_WIDTH || c == '\n') {
        cursor_x = 0;
        cursor_y++;

        if (cursor_y >= VGA_HEIGHT) vga_scroll(1);
    }

    vga_setCursor(cursor_x, cursor_y);
}

void vga_setColor(u8 fg, u8 bg) {
    color = fg + (bg << 4);
}

void vga_write(const char* str) {
    int i = 0;
    while (str[i] != 0) {
        vga_writeChar(str[i]);
        i++;
    }
}

void vga_print(const char* str) {
    vga_write(str);
    vga_writeChar('\n');
}

void vga_writeByte(u8 value) {
    vga_writeChar(hex[value >> 4]);
    vga_writeChar(hex[value & 0xf]);
}

void vga_writeWord(u16 value) {
    vga_writeByte(value >> 8);
    vga_writeByte(value & 0xff);
}

void vga_writeDWord(u32 value) {
    vga_writeWord(value >> 16);
    vga_writeWord(value & 0xffff);
}

char writeInteger_value[20];

void vga_writeInteger(int value) {
    if (value == 0){
        vga_writeChar('0');
        return;
    }

    if (value < 0) {
        vga_writeChar('-');
        value = -value;
    }
    
    int i;
    for (i = 0; value != 0; i++) {
        writeInteger_value[sizeof(writeInteger_value) - i - 2] = hex[value % 10];
        value /= 10;
    }

    writeInteger_value[sizeof(writeInteger_value) - 1] = 0;

    vga_write(writeInteger_value + sizeof(writeInteger_value) - i - 1);
}

void vga_clear() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vram[i]     = ' ';
        vram[i + 1] = color;
    }
}

void vga_setCursor(u8 x, u8 y) {
    cursor_x = x;
    cursor_y = y;

    u16 pos = y * VGA_WIDTH + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (u16) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (u16) ((pos >> 8) & 0xFF));
}

u8 vga_getCursorX() {
    return cursor_x;
}

u8 vga_getCursorY() {
    return cursor_y;
}

void vga_showCursor() {
    outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | VGA_CURSOR_START_LINE);
    
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | VGA_CURSOR_END_LINE);
}

void vga_hideCursor() {
    outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void vga_hexDump(u8* buffer, int size) {
    int line = size / 16 + (size % 16 != 0);

    for (int i = 0; i < line; i++) {
        vga_writeDWord(i * 16);
        vga_write("  ");

        int a = 0;
        for (int b = i * 16; b < (i + 1) * 16 && b < size; b++) {
            vga_writeByte(buffer[b]);
            vga_writeChar(' ');

            a++;
        }

        for (int s = 0; s < 16 - a; s++)
            vga_write("   ");

        vga_writeChar(' ');

        for (int b = i * 16; b < (i + 1) * 16 && b < size; b++)
            vga_writeChar(buffer[b]);

        vga_writeChar('\n');
    }
}

void vga_init() {
    vga_setCursor(0, 0);
    vga_showCursor();
}