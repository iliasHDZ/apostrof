#include "terminal.h"
#include <apostrof.h>

unsigned int fb;

unsigned int width;
unsigned int height;

unsigned int cursor_x;
unsigned int cursor_y;

unsigned char fg_color = 0x0F;
unsigned char bg_color = 0x00;

typedef enum EscapeState {
    NONE,
    ESCAPE,
    C1
} EscapeState;

EscapeState escape_state = NONE;
char c1 = 0;

char csi_buf[32];
char csi_buf_len = 0;

int  csi_prm[2];
char csi_prm_len = 0;
int  csi_num = 0;

static char term_isCSIByte(char c) {
    char csib[] = "0123456789:;<=>?!\"#$%&'()*+,-./";

    for (int i = 0; i < sizeof(csib); i++)
        if (c == csib[i])
            return 1;

    return 0;
}

static char is_number(char c) {
    return c >= '0' && c <= '9';
}

int term_init() {
    fb = open("/dev/fb", O_RDWR);
    if (fb == 0) return 0;

    width  = video_get(VIDEO_WIDTH);
    height = video_get(VIDEO_HEIGHT);

    term_setCursor(0, 0);
    return 1;
}

unsigned int clamp(int x, int min, int max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

static void term_parseCSI() {
    int begin = 0;
    if (csi_buf[0] == '?') begin = 1;

    csi_num = 0;
    csi_prm_len = 0;

    int i;

    for (i = begin; i < csi_buf_len - 1; i++) {
        char c = csi_buf[i];
        if (c == ';') {
            if (begin == i)
                continue;
            
            if (csi_buf_len == 1)
                return;

            csi_prm[csi_prm_len++] = csi_num;
            csi_num = 0;
        } else if (is_number(c))
            csi_num = csi_num * 10 + (c - '0');
        else return;
    }
    
    if (i > begin) {
        if (csi_buf_len == 1)
            return;
            
        csi_prm[csi_prm_len++] = csi_num;
    }

    int prm1, prm2, start, end;
    char* buffer;

    int cursor = cursor_y * width + cursor_x;

    char cr = csi_buf[csi_buf_len - 1];

    switch(cr) {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
        prm1 = csi_prm_len >= 1 ? csi_prm[0] : 1;
        
        switch(cr) {
        case 'A': cursor_y -= prm1; break;
        case 'B': cursor_y += prm1; break;
        case 'C': cursor_x += prm1; break;
        case 'D': cursor_x -= prm1; break;
        case 'E': cursor_y += prm1; cursor_x = 0; break;
        case 'F': cursor_y -= prm1; cursor_x = 0; break;
        case 'G': cursor_x = prm1 - 1; break;
        }

        cursor_x = clamp(cursor_x, 0, width - 1);
        cursor_y = clamp(cursor_y, 0, height - 1);
        break;
    case 'H':
        prm1 = csi_prm_len >= 1 ? csi_prm[0] : 1;
        prm2 = csi_prm_len >= 2 ? csi_prm[1] : 1;

        cursor_y = clamp(prm1 - 1, 0, height - 1);
        cursor_x = clamp(prm2 - 1, 0, width - 1);
        break;
    case 'J':
    case 'K':
        if (csi_prm_len == 0)
            return;

        prm1 = csi_prm[0];

        if (cr == 'J')
            if (prm1 == 0) {
                start = cursor;
                end   = width * height;
            } else if (prm1 == 1) {
                start = 0;
                end   = cursor;
            } else if (prm1 == 2 || prm1 == 3) {
                start = 0;
                end   = width * height;
            } else
                return;
        else if (cr == 'K') {
            if (prm1 == 0) {
                start = cursor;
                end   = cursor_y * width + width;
            } else if (prm1 == 1) {
                start = cursor_y * width;
                end   = cursor;
            } else if (prm1 == 2) {
                start = cursor_y * width;
                end   = start + width;
            } else
                return;
        }

        dbg_writeInteger(cursor);
        dbg_write("(");
        dbg_writeInteger(start);
        dbg_write(", ");
        dbg_writeInteger(end);
        dbg_write(")\n");

        buffer = (char*)malloc((end - start) * 2);

        for (int i = 0; i < (end - start); i++) {
            buffer[i * 2] = ' ';
            buffer[i * 2 + 1] = fg_color | bg_color << 4;
        }

        seek(fb, start * 2, SEEK_SET);
        write(fb, buffer, (end - start) * 2);

        free(buffer);
        break;
    case 'S':
        prm1 = csi_prm_len >= 1 ? csi_prm[0] : 1;

        term_scroll(prm1);
        break;
    }
}

static void term_writeEscaped(char c) {
    if (escape_state == ESCAPE) {
        switch (c) {
        case '[':
            c1 = c;
            csi_buf_len = 0;
            escape_state = C1;
            break;
        
        default:
            break;
        }
        return;
    }

    if (c1 == '[') {
        if (csi_buf_len >= sizeof(csi_buf)) {
            escape_state = NONE;
            return;
        }

        csi_buf[csi_buf_len++] = c;
        if (!term_isCSIByte(c)) {
            term_parseCSI();
            term_updateCursor();
            escape_state = NONE;
        }

        return;
    }
}

void term_write(char c) {
    char buffer[2];

    if (escape_state != NONE) {
        term_writeEscaped(c);
        return;
    }

    switch (c) {
    case 0x07: // BELL
        break;
    case 0x08: // BACKSPACE
        if (cursor_x > 0) cursor_x--;
        break;
    case 0x09: // TAB
        cursor_x += 8 - cursor_x % 8;
        break;
    case 0x0A: // LINEFEED
        cursor_y++;
        cursor_x = 0;
        break;
    case 0x0D: // CARRIAGE RETURN
        cursor_x = 0;
        break;
    case 0x1B: // ESCAPE
        escape_state = ESCAPE;
        break;
    default:
        seek(fb, (cursor_y * width + cursor_x) * 2, SEEK_SET);
        buffer[0] = c;
        buffer[1] = (fg_color & 0x0F) | ((bg_color & 0x0F) << 4);
        write(fb, &buffer, 2);

        cursor_x++;
    }

    term_updateCursor();
}

void term_scroll(unsigned short lines) {
    char* buffer = malloc(width * height * 2);

    seek(fb, 0, SEEK_SET);
    read(fb, buffer, width * height * 2);

    for (int y = 0; y < height; y++) {
        if (y + lines < height)
            memcpy(buffer + y * width * 2, buffer + (y + lines) * width * 2, width * 2);
        else
            for (int x = 0; x < width; x++) {
                buffer[(y * width + x) * 2] = 0x00;
                buffer[(y * width + x) * 2 + 1] = (bg_color & 0x0F) << 4;
            }
    }

    cursor_y -= lines;
    if (cursor_y < 0) cursor_y = 0;

    seek(fb, 0, SEEK_SET);
    write(fb, buffer, width * height * 2);
    free(buffer);
}

void term_updateCursor() {
    if (cursor_x >= width) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= height)
        term_scroll(1);

    term_setCursor(cursor_x, cursor_y);
}

void term_setCursor(unsigned int x, unsigned int y) {
    video_set(VIDEO_CUR_X, x);
    video_set(VIDEO_CUR_Y, y);
    cursor_x = x;
    cursor_y = y;
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