#include <apostrof.h>

unsigned int vga_open() {
    return syscall(0x80, 0, 0, 0);
}

int main() {
    write(1, "TEST!", 5);

    while (1);

    return 0;
}