unsigned int syscall(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx) {
    asm("int $0x1C;" : "=a" (eax) : "a" (eax), "b" (ebx), "c" (ecx), "d" (edx));
    return eax;
}

unsigned int vga_open() {
    return syscall(0x80, 0, 0, 0);
}

void* malloc(unsigned int size) {
    return (void*)syscall(0x30, size, 0, 0);
}

void free(void* block) {
    syscall(0x31, (unsigned int)block, 0, 0);
}

void* realloc(void* block, unsigned int size) {
    return (void*)syscall(0x32, (unsigned int)block, size, 0);
}

int fwrite(unsigned int fd, char* buffer, unsigned int size) {
    return (int)syscall(0x11, fd, (unsigned int)buffer, size);
}

int write_char(int fd, char c, char color) {
    char buffer[] = {c, color};

    fwrite(fd, buffer, 2);
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

// .rodata
const char* test1 = "TEST1 ";

// .data
char* test2 = "TEST2 ";

// .bss
char* test3;

int start() {
    fd = vga_open();
    if (fd == 0) return 1;

    print(test1);
    print(test2);

    test2 = "_TEST2 ";

    print(test2);

    test3 = "TEST3 ";

    print(test3);

    // would have liked for this to give an exception, but the
    // sections are bunched together in one page which can only
    // has a writable bit for the whole page.
    test1 = "EXCEPTION? ";

    for (;;);

    return 0;
}