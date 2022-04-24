#include "fd.h"
#include "../kmm.h"

#include "../vga.h"

fd __void_stream = {FD_MAGIC, FD_VOID, 0, 0, 0, 0};
fd* void_stream  = &__void_stream;

fd* fd_createBuffer(char* buffer, u32 buffer_size, u8 writable) {
    fd* fd = kmalloc(sizeof(struct fd));
    if (fd == 0) return 0;

    fd->magic = FD_MAGIC;
    fd->type = writable ? FD_RW_FBUFFER : FD_RO_FBUFFER;

    fd->buffer      = buffer;
    fd->buffer_size = buffer_size;

    fd->cursor = 0;
}

fd* fd_createOutStream(fd* pipe) {
    fd* fd = kmalloc(sizeof(struct fd));
    if (fd == 0) return 0;

    fd->magic = FD_MAGIC;
    fd->type = FD_OUT_STREAM;

    fd->buffer      = 0;
    fd->buffer_size = 0;

    if (pipe != 0)
        fd->pipe = pipe;
    else
        fd->pipe = void_stream;

    fd->cursor = 0;
}

fd* fd_createInStream() {
    fd* fd = kmalloc(sizeof(struct fd));
    if (fd == 0) return 0;

    fd->magic = FD_MAGIC;
    fd->type = FD_IN_STREAM;

    fd->buffer      = 0;
    fd->buffer_size = 0;

    fd->cursor = 0;
}

u32 fd_getSize(fd* fd) {
    return fd->buffer_size;
}

u32 fd_read(fd* fd, char* buffer, u32 size) {
    if (fd->type == FD_RO_FBUFFER || fd->type == FD_RW_FBUFFER) {
        if (fd->cursor + size >= fd->buffer_size)
            size = fd->buffer_size - fd->cursor;

        memcpy(buffer, fd->buffer + fd->cursor, size);
        fd->cursor += size;
    
        return size;
    } else if (fd->type == FD_IN_STREAM) {
        u32 written = size > fd->buffer_size ? fd->buffer_size : size;

        memcpy(buffer, fd->buffer, written);
        memcpy(fd->buffer, fd->buffer + written, fd->buffer_size - written);

        fd->buffer_size -= written;
        return written;
    } else
        return 0;
}

u32 fd_istream_write(fd* fd, char* buffer, u32 size) {
    if (fd->type == FD_IN_STREAM) {

        if (fd->buffer == 0) {
            fd->buffer = kmalloc(size);
            if (fd->buffer == 0) return 0;

            memcpy(fd->buffer, buffer, size);
            fd->buffer_size = size;
            return size;
        }

        u8* nbuffer = krealloc(fd->buffer, fd->buffer_size + size);
        if (nbuffer == 0) return 0;

        memcpy(nbuffer + fd->buffer_size, buffer, size);
        fd->buffer = nbuffer;
        fd->buffer_size += size;
        
        return size;
    } else if (fd->type == FD_VOID)
        return size;
    
    return 0;
}

u32 fd_write(fd* fd, char* buffer, u32 size) {
    if (fd->type == FD_RW_FBUFFER) {
        if (fd->cursor + size >= fd->buffer_size)
            size = fd->buffer_size - fd->cursor;

        memcpy(fd->buffer + fd->cursor, buffer, size);
        fd->cursor += size;
    
        return size;
    } else if (fd->type == FD_OUT_STREAM) {
        if (fd->pipe != 0)
            return fd_istream_write(fd->pipe, buffer, size);
    }

    return 0;
}

u32 fd_tell(fd* fd) {
    if (fd->type == FD_RO_FBUFFER || fd->type == FD_RW_FBUFFER)
        return fd->cursor;
    else
        return 0;
}

u32 fd_seek(fd* fd, int offset, int whence) {
    if (fd->type == FD_RO_FBUFFER || fd->type == FD_RW_FBUFFER) {
        int cursor;
        switch (whence) {
        case SEEK_SET: cursor = 0; break;
        case SEEK_CUR: cursor = fd->cursor; break;
        case SEEK_END: cursor = fd->buffer_size; break;
        default: return 1;
        }

        cursor += offset;
        if (cursor >= fd->buffer_size) return 1;

        fd->cursor = cursor;
    } else
        return 1;

    return 0;
}

void fd_close(fd* fd) {
    if (fd->type == FD_IN_STREAM || fd->type == FD_OUT_STREAM)
        kfree(fd->buffer);

    kfree(fd);
}