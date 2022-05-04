#include "dbg.h"
#include "io.h"

static const char* hex = "0123456789ABCDEF";

void dbg_write(const char *s) {
    while (*s)
        outb(0xe9, *(s++));
}

void dbg_writeChar(char s) {
    outb(0xe9, s);
}

void dbg_writeByte(unsigned char value) {
    dbg_writeChar(hex[value >> 4]);
    dbg_writeChar(hex[value & 0xf]);
}

void dbg_writeWord(unsigned short value) {
    dbg_writeByte(value >> 8);
    dbg_writeByte(value & 0xff);
}

void dbg_writeDWord(unsigned int value) {
    dbg_writeWord(value >> 16);
    dbg_writeWord(value & 0xffff);
}

void dbg_hexDump(unsigned char* buffer, int size) {
    int line = size / 16 + (size % 16 != 0);

    for (int i = 0; i < line; i++) {
        dbg_writeDWord(i * 16);
        dbg_write("  ");

        int a = 0;
        for (int b = i * 16; b < (i + 1) * 16 && b < size; b++) {
            dbg_writeByte(buffer[b]);
            dbg_writeChar(' ');

            a++;
        }

        for (int s = 0; s < 16 - a; s++)
            dbg_write("   ");

        dbg_writeChar(' ');

        for (int b = i * 16; b < (i + 1) * 16 && b < size; b++) {
            char c = buffer[b];
            if (c >= 32 && c < 255)
                dbg_writeChar(c);
            else
                dbg_writeChar('.');
        }

        dbg_writeChar('\n');
    }
}