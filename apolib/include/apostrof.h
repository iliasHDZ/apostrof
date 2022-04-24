#pragma once

unsigned int syscall(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx);

void exit(int code);

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