#pragma once

#ifdef __cplusplus
extern "C" {
#endif

unsigned int syscall(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx);

void exit(int code);

// UTIL

void* memcpy(void* dst, const void* src, unsigned int size);

void* memset(void* ptr, int value, unsigned int size);

unsigned int strlen(const char* str);

// FILE DESCRIPTORS

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

#define STDIN  0
#define STDOUT 1
#define STDERR 2

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2

unsigned int open(const char* path, int flags);

unsigned int read(unsigned int fd, void* buffer, unsigned int count);

unsigned int write(unsigned int fd, void* buffer, unsigned int count);

unsigned int seek(unsigned int fd, unsigned int offset, int whence);

unsigned int tell(unsigned int fd);

// MEMORY MANAGEMENT

void* malloc(unsigned int size);

void free(void* block);

void* realloc(void* block, unsigned int size);

// PROCESSES

unsigned int process_create(const char* path);

unsigned int process_getStdStream(unsigned int pid, unsigned int stream);

unsigned int process_current();

unsigned int process_createWithStdout(const char* path, unsigned int* fd);

// VIDEO

#define VIDEO_MODE   0
#define VIDEO_WIDTH  1
#define VIDEO_HEIGHT 2
#define VIDEO_CURSOR 3
#define VIDEO_CUR_X  4
#define VIDEO_CUR_Y  5

#define VIDEO_TEXT_MODE 0

unsigned int video_get(int global);

unsigned int video_set(int global, unsigned int value);

// DEBUG

void dbg_write(const char *s);

void dbg_writeLen(const char *s, unsigned int len);

void dbg_writeChar(char s);

void dbg_writeByte(unsigned char value);

void dbg_writeWord(unsigned short value);

void dbg_writeDWord(unsigned int value);

void dbg_writeInteger(int value);

void dbg_hexDump(unsigned char* buffer, int size);

#ifdef __cplusplus
}
#endif