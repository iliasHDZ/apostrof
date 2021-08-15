unsigned int syscall(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx) {
    asm("int $0x1C;" : "=a" (eax) : "a" (eax), "b" (ebx), "c" (ecx), "d" (edx));
    return eax;
}

int vga_open() {
    return syscall(0x80, 0, 0, 0);
}

int fwrite(int fd, char* buffer, int size) {
    return syscall(0x11, fd, buffer, size);
}

int write_char(int fd, char c, char color) {
    char buffer[] = {c, color};

    fwrite(fd, buffer, 2);
}

int start() {
    int fd = vga_open();
    if (fd == 0) return 1;

    write_char(fd, 'H', 0x0f);
    write_char(fd, 'e', 0x0f);
    write_char(fd, 'l', 0x0f);
    write_char(fd, 'l', 0x0f);
    write_char(fd, 'o', 0x0f);
    write_char(fd, '!', 0x0f);

    return 0;
}