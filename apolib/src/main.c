#include <apostrof.h>

// Program entry point
int main();

void __apo_entry() {
    int err = main();

    for (;;);

    exit(err);
}

unsigned int syscall(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx) {
    asm("int $0x1C;" : "=a" (eax) : "a" (eax), "b" (ebx), "c" (ecx), "d" (edx));
    return eax;
}

void exit(int code) {
    syscall(0x01, code, 0, 0);
}

unsigned int open(const char* path, int flags) {
    return syscall(0x60, (unsigned int)path, flags, 0);
}

unsigned int read(unsigned int fd, void* buffer, unsigned int count) {
    return syscall(0x10, fd, (unsigned int)buffer, count);
}

unsigned int write(unsigned int fd, void* buffer, unsigned int count) {
    return syscall(0x11, fd, (unsigned int)buffer, count);
}

unsigned int seek(unsigned int fd, unsigned int offset, int whence) {
    return syscall(0x12, fd, offset, (unsigned int)whence);
}

unsigned int tell(unsigned int fd) {
    return syscall(0x13, fd, 0, 0);
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

unsigned int process_create(const char* path) {
    return syscall(0x40, (unsigned int)path, 0, 0);
}

unsigned int process_getStdStream(unsigned int pid, unsigned int stream) {
    return syscall(0x41, pid, stream, 0);
}

unsigned int process_current() {
    return syscall(0x42, 0, 0, 0);
}

unsigned int process_createWithStdout(const char* path, unsigned int* fd) {
    return syscall(0x43, (unsigned int)path, (unsigned int)fd, 0);
}

unsigned int video_get(int global) {
    return syscall(0x80, global, 0, 0);
}

unsigned int video_set(int global, unsigned int value) {
    return syscall(0x81, global, value, 0);
}