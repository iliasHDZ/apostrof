#pragma once

#include "../utils.h"

typedef enum {
    PARTITION,
    DRIVE_IDE
} DRIVE_TYPE;

typedef struct {
    DRIVE_TYPE type;

    char* device_name; // IDE = Model name
    u32   size;        // Size in sectors (1 sector = 512b)
} drive;

typedef struct drive_partition {
    DRIVE_TYPE type;

    char*  device_name;
    u32    size;

    drive* parent;

    u32    start_sector;
    u32    end_sector;
} drive_partition;

typedef struct ide_drive {
    DRIVE_TYPE type;

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

drive* drive_getDevice(int index);

drive_partition* drive_getPartitionByIndex(int index);

void drive_addPartition(char* name, drive* parent, u32 start_sector, u32 end_sector);

const char* drive_path(drive* d);

u8 drive_read(drive* d, u32 sector, u8 sectorcount, u8* dst);

void drive_init();