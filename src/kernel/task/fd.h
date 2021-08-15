#pragma once

#include "../utils.h"

#define FD_RO_FBUFFER 1 // Read-Only  Fixed-size buffer
#define FD_RW_FBUFFER 2 // Read-Write Fixed-size buffer
#define FD_IN_STREAM  3 // Input  Stream
#define FD_OUT_STREAM 4 // Output Stream

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

typedef struct fd {
    u8 type;
    
    char* buffer;
    u32   buffer_size;

    u8 cursor; // Buffer-Only
} fd;

fd* fd_createBuffer(char* buffer, u32 buffer_size, u8 writable);

fd* fd_createOutStream();

fd* fd_createInStream();

u32 fd_getSize(fd* fd);

u32 fd_read(fd* fd, char* buffer, u32 size);

u32 fd_write(fd* fd, char* buffer, u32 size);

u32 fd_tell(fd* fd);

u32 fd_seek(fd* fd, int offset, int whence);