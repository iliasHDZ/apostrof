#pragma once

#include "../utils.h"

typedef enum {
    STORAGE_IDE
} STORAGE_TYPE;

typedef struct {
    STORAGE_TYPE type;

    char* device_name; // IDE = Model name
    u32   size;        // Size in sectors (1 sector = 512b)
} storage_dev;

typedef struct {
    STORAGE_TYPE type;

    char* device_name;
    u32   size;

    u8    atapi;

    u8    present;
    u8    channel;
    u8    drive;

    u16   signature;
    u16   capabilities;
    u32   command_sets;
} ide_drive;

storage_dev* storage_getDevice(int index);

const char* storage_path(storage_dev* d);

u8 storage_read(storage_dev* d, u32 sector, u8 sectorcount, u8* dst);

void storage_init();