#include <apostrof.h>

void dbg_writeChar(char s) {
    syscall(0x120, s, 0, 0);
}

void dbg_write(const char *s) {
    while (*s)
        dbg_writeChar(*(s++));
}

void dbg_writeLen(const char *s, unsigned int len) {
    for (unsigned int i = 0; i < len; i++)
        dbg_writeChar(*(s + i));
}

void dbg_writeByte(unsigned char value) {
    char hex[] = "0123456789ABCDEF";
    
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

static char writeInteger_value[20];

void dbg_writeInteger(int value) {
    char hex[] = "0123456789ABCDEF";

    if (value == 0){
        dbg_writeChar('0');
        return;
    }

    if (value < 0) {
        dbg_writeChar('-');
        value = -value;
    }
    
    int i;
    for (i = 0; value != 0; i++) {
        writeInteger_value[sizeof(writeInteger_value) - i - 2] = hex[value % 10];
        value /= 10;
    }

    writeInteger_value[sizeof(writeInteger_value) - 1] = 0;

    dbg_write(writeInteger_value + sizeof(writeInteger_value) - i - 1);
}

void dbg_hexDump(unsigned char* buffer, int size) {
    int line = size / 16 + (size % 16 != 0);

    for (int i = 0; i < line; i++) {
        dbg_writeDWord(buffer + i * 16);
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