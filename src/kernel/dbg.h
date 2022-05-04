#pragma once

void dbg_write(const char *s);

void dbg_writeLen(const char *s, unsigned int len);

void dbg_writeChar(char s);

void dbg_writeByte(unsigned char value);

void dbg_writeWord(unsigned short value);

void dbg_writeDWord(unsigned int value);

void dbg_writeInteger(int value);

void dbg_hexDump(unsigned char* buffer, int size);