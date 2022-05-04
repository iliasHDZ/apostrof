#include <apostrof.h>

unsigned int vga_open() {
    return syscall(0x80, 0, 0, 0);
}

void print(const char* str) {
    write(1, str, strlen(str));
}

int main() {
    print("AAAAAAAAAA\nAAAAAAAAAA\nAAAAAAAAAA\nAAAAAAAAAA\nAAAAAAAAAA\nAAAAAAAAAA\nAAAAAAAAAA\nAAAAAAAAAA\n"
          "\x1B[3;5H\x1B[2K");

    while (1);

    return 0;
}