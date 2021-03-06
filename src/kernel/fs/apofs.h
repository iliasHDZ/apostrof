#pragma once

#include "../utils.h"
#include "../drv/drive.h"

typedef struct {
    drive* device;

    u32 bitmap_base;
    u32 table_base;
    u32 file_base;
    u16 desc_size;

    u8* bitmap;
    u32 bitmap_size;
} apo_fs;

typedef struct {
    u8  dir;
    u32 parent;
    u64 creation_date;
    u64 modified_date;
    u16 attributes;
} apofs_fileinfo;

extern apo_fs* current_root;

apo_fs* apofs_openDevice(drive* d);

u16 apofs_lastError();

u8 apofs_getFileName(apo_fs* fs, u32 file_id, char* name_out, int len_out);

u8 apofs_getFileInfo(apo_fs* fs, u32 file_id, apofs_fileinfo* info_out);

u32 apofs_getChild(apo_fs* fs, u32 file_id, const char* child_name);

u32 apofs_getFile(apo_fs* fs, const char* path);

u32 apofs_getFileSize(apo_fs* fs, u32 file_id);

u32 apofs_read(apo_fs* fs, u32 file_id, u8* dst, u32 offset, u32 count);

void apofs_init();