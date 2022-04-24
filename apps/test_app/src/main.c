#include <apostrof.h>

const char* hex = "0123456789ABCDEF";

int write_char(int fd, char c, char color) {
    char buffer[] = {c, color};

    write(fd, buffer, 2);
}

int fd;

void print(const char* str) {
    while (*str)
        write_char(fd, *(str++), 0x0F);
}

void* memcpy(void* dst, const void* src, unsigned int size) {
    for (unsigned int i = 0; i < size; i++)
        ((unsigned char*)dst)[i] = ((unsigned char*)src)[i];

    return dst;
}

void* memset(void* ptr, int value, unsigned int size) {
    for (unsigned int i = 0; i < size; i++)
        ((unsigned char*)ptr)[i] = (unsigned char)value;

    return ptr;
}

void vga_writeByte(unsigned char value) {
    write_char(fd, hex[value >> 4], 0x0f);
    write_char(fd, hex[value & 0xf], 0x0f);
}

void vga_writeWord(unsigned short value) {
    vga_writeByte(value >> 8);
    vga_writeByte(value & 0xff);
}

void vga_writeDWord(unsigned int value) {
    vga_writeWord(value >> 16);
    vga_writeWord(value & 0xffff);
}

int main() {
    fd = open("/dev/fb", O_RDWR);
    if (fd == 0) return 1;

    unsigned int stdout;

    unsigned int pid = process_createWithStdout("/test_app2", &stdout);
    if (pid == 0) return 1;

    while (1) {
        char c;
        if (read(stdout, &c, 1) == 1)
            write_char(fd, c, 0x0F);
    }

    return 0;
}