#pragma once

void dbg_write(const char *s);

void dbg_writeByte(unsigned char value);

void dbg_writeWord(unsigned short value);

void dbg_writeDWord(unsigned int value);

void dbg_hexDump(unsigned char* buffer, int size);