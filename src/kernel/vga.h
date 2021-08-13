#pragma once

#include "utils.h"

#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25

#define VGA_BLACK       0x0
#define VGA_BLUE        0x1
#define VGA_GREEN       0x2
#define VGA_CYAN        0x3
#define VGA_RED         0x4
#define VGA_MAGENTA     0x5
#define VGA_BROWN       0x6
#define VGA_LIGHT_GRAY  0x7
#define VGA_DARK_GRAY   0x8
#define VGA_LIGHT_BLUE  0x9
#define VGA_LIGHT_GREEN 0xA
#define VGA_LIGHT_CYAN  0xB
#define VGA_LIGHT_RED   0xC
#define VGA_PINK        0xD
#define VGA_YELLOW      0xE
#define VGA_WHITE       0xF

#define VGA_CURSOR_START_LINE 14
#define VGA_CURSOR_END_LINE   15

extern u8 cursor_y;

void vga_setChar(char c, u8 x, u8 y);

void vga_scroll(int count);

void vga_writeChar(char c);

void vga_setColor(u8 fg, u8 bg);

//void vga_write(const char* str);

void vga_print(const char* str);

void vga_writeByte(u8 value);

void vga_writeWord(u16 value);

void vga_writeDWord(u32 value);

int vga_seek(int offset, int whence);

int vga_tell();

int vga_read(char* buffer, int size);

int vga_write(const char* buffer, int size);

int vga_get(int global);

int vga_set(int global, int value);

void vga_writeInteger(int value);

void vga_clear();

void vga_setCursor(u8 x, u8 y);

u8 vga_getCursorX();

u8 vga_getCursorY();

void vga_showCursor();

void vga_hideCursor();

void vga_hexDump(u8* buffer, int size);

void vga_init();