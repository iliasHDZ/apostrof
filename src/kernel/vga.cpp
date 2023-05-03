extern "C" {
#include "vga.h"

#include "io.h"
#include "kmm.h"
}

#include "task/task.hpp"

char* vram  = (char*)0xb8000;
u8 color    = VGA_WHITE;

u8 cursor_x = 0;
u8 cursor_y = 0;

u8 cursor_visible = 0;

static const char* hex = "0123456789ABCDEF";

u32 ptr_cursor  = 0;
u32 vram_size   = VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT * 2;

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

#define VGA_MODE   0
#define VGA_WIDTH  1
#define VGA_HEIGHT 2
#define VGA_CURSOR 3
#define VGA_CUR_X  4
#define VGA_CUR_Y  5

#define VGA_TEXT_MODE 0

void vga_setChar(char c, u8 x, u8 y) {
    if (x < 0 || x >= VGA_TEXT_WIDTH ||
        y < 0 || y >= VGA_TEXT_HEIGHT) return;

    int offset = (y * VGA_TEXT_WIDTH + x) * 2;

    vram[offset]     = c;
    vram[offset + 1] = color;
}

void vga_scroll(int count) {
    for (int x = 0; x < VGA_TEXT_WIDTH; x++)
        for (int y = 0; y < VGA_TEXT_WIDTH; y++) {
            int get_y = y + count;

            if (get_y >= VGA_TEXT_HEIGHT) vga_setChar(' ', x, y);
            else {
                int src = (get_y * VGA_TEXT_WIDTH + x) * 2;
                int dst = (y * VGA_TEXT_WIDTH + x) * 2;

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
    if (cursor_x >= VGA_TEXT_WIDTH || c == '\n') {
        cursor_x = 0;
        cursor_y++;

        if (cursor_y >= VGA_TEXT_HEIGHT) vga_scroll(1);
    }

    vga_setCursor(cursor_x, cursor_y);
}

void vga_setColor(u8 fg, u8 bg) {
    color = fg + (bg << 4);
}

void vga_writeText(const char* str) {
    int i = 0;
    while (str[i] != 0) {
        vga_writeChar(str[i]);
        i++;
    }
}

void vga_print(const char* str) {
    vga_writeText(str);
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

int vga_seek(int offset, int whence) {
    int cursor;
    switch (whence) {
    case SEEK_SET: cursor = 0; break;
    case SEEK_CUR: cursor = ptr_cursor; break;
    case SEEK_END: cursor = vram_size; break;
    default: return 1;
    }

    cursor += offset;
    if (cursor >= vram_size) return 1;

    ptr_cursor = cursor;
    return 0;
}

int vga_tell() {
    return ptr_cursor;
}

int vga_read(char* buffer, int size) {
    if (ptr_cursor + size >= vram_size)
        size = vram_size - ptr_cursor;

    memcpy(buffer, vram + ptr_cursor, size);
    ptr_cursor += size;
    
    return size;
}

int vga_write(const char* buffer, int size) {
    if (ptr_cursor + size >= vram_size)
        size = vram_size - ptr_cursor;

    memcpy(vram + ptr_cursor, buffer, size);
    ptr_cursor += size;

    return size;
}

int vga_get(int global) {
    switch (global)
    {
    case VGA_MODE:   return VGA_TEXT_MODE;
    case VGA_WIDTH:  return VGA_TEXT_WIDTH;
    case VGA_HEIGHT: return VGA_TEXT_HEIGHT;
    case VGA_CURSOR: return cursor_visible;
    case VGA_CUR_X:  return cursor_x;
    case VGA_CUR_Y:  return cursor_y;
    default: return -1;
    }
}

int vga_set(int global, int value) {
    switch (global)
    {
    case VGA_MODE:
    case VGA_WIDTH:
    case VGA_HEIGHT: return -1;
    case VGA_CURSOR:
        if (value) vga_showCursor();
        else vga_hideCursor();
        return 0;
    case VGA_CUR_X:
        vga_setCursor(value, cursor_y);
        return 0;
    case VGA_CUR_Y:
        vga_setCursor(cursor_x, value);
        return 0;
    default: return -1;
    }
}

fd* vga_open() {
    fd* fd = fd_createBuffer(vram, vram_size, 1);
    if (fd == 0) return 0;

    apo::Task* task = apo::Task::current();
    if (task == 0) {
        kfree(fd); // fd_free();
        return 0;
    }

    if (task->attach(fd) != 0) {
        kfree(fd); // fd_free();
        return 0;
    }

    return fd;
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

    vga_writeText(writeInteger_value + sizeof(writeInteger_value) - i - 1);
}

void vga_clear() {
    for (int i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT * 2; i += 2) {
        vram[i]     = ' ';
        vram[i + 1] = color;
    }
}

void vga_setCursor(u8 x, u8 y) {
    cursor_x = x;
    cursor_y = y;

    u16 pos = y * VGA_TEXT_WIDTH + x;
 
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

    cursor_visible = 1;
}

void vga_hideCursor() {
    outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);

    cursor_visible = 0;
}

void vga_hexDump(u8* buffer, int size) {
    int line = size / 16 + (size % 16 != 0);

    for (int i = 0; i < line; i++) {
        vga_writeDWord(i * 16);
        vga_writeText("  ");

        int a = 0;
        for (int b = i * 16; b < (i + 1) * 16 && b < size; b++) {
            vga_writeByte(buffer[b]);
            vga_writeChar(' ');

            a++;
        }

        for (int s = 0; s < 16 - a; s++)
            vga_writeText("   ");

        vga_writeChar(' ');

        for (int b = i * 16; b < (i + 1) * 16 && b < size; b++) {
            char c = buffer[b];
            if (c >= 32 && c < 255)
                vga_writeChar(c);
            else
                vga_writeChar('.');
        }

        vga_writeChar('\n');
    }
}

void vga_init() {
    vga_setCursor(0, 0);
    vga_showCursor();
}