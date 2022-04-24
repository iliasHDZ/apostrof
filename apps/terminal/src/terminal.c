#include "terminal.h"
#include <apostrof.h>

unsigned int fb;

unsigned int width;
unsigned int height;

unsigned int cursor_x;
unsigned int cursor_y;

unsigned char fg_color = 0x0F;
unsigned char bg_color = 0x00;

int term_init() {
    fb = open("/dev/fb", O_RDWR);
    if (fb == 0) return 0;

    width  = video_get(VIDEO_WIDTH);
    height = video_get(VIDEO_HEIGHT);

    term_setCursor(0, 0);
    return 1;
}

void term_write(char c) {
    char buffer = {c, 0x0E};//(fg_color & 0x0F) | ((bg_color & 0x0F) << 4)};
    write(fb, &buffer, 2);

    cursor_x++;

    if (c == '\n') {
        cursor_y++;
        cursor_x = 0;
    }
    
    term_setCursor(cursor_x, cursor_y);
}

void term_setCursor(unsigned int x, unsigned int y) {
    video_set(VIDEO_CUR_X, x);
    video_set(VIDEO_CUR_Y, y);
    cursor_x = x;
    cursor_y = y;

    seek(fb, (x + y * width) * 2, SEEK_SET);
}

int term_run(const char* app) {
    unsigned int stdout;

    unsigned int pid = process_createWithStdout(app, &stdout);
    if (pid == 0) return 1;

    while (1) {
        char c;
        if (read(stdout, &c, 1) == 1)
            term_write(c);
    }
}