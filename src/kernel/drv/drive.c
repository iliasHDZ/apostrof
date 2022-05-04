#include "drive.h"

#include "../kmm.h"
#include "ide.h"

#include "gpt.h"

static PARRAY drives;
static PARRAY partitions;

drive* drive_getDevice(int index) {
    return (drive*)parray_get(&drives, index);
}

drive_partition* drive_getPartitionByIndex(int index) {
    return (drive_partition*)parray_get(&partitions, index);
}

char storage_path_str[20] = "/dev/";

const char* drive_path(drive* d) {
    if (d == 0) return "unknown";

    if (d->type == DRIVE_IDE) {
        ide_drive* ide = (ide_drive*)d;

        memcpy(storage_path_str + 5, "hdx", 4);
        storage_path_str[7] = ("abcd")[ide->channel << 1 | ide->drive];

        return storage_path_str;
    } else return "unknown";
}

u8 drive_read(drive* d, u32 sector, u8 sectorcount, u8* dst) {
    if (d->type == PARTITION && 0) {
        drive_partition* part = (drive_partition*) d;

        if (sector + sectorcount > part->end_sector)
            return 1;

        return drive_read(part->parent, sector + part->start_sector, sectorcount, dst);
    } else if (d->type == DRIVE_IDE) {
        ide_drive* ide = (ide_drive*) d;

        if (!ide->atapi)
            return ide_ata_read(ide, sector, sectorcount, dst);
        else return 1;
    } else return 1;
}

u8 drive_add(drive* d) {
    //gpt_read(d);
    return parray_push(&drives, d);
}

u8 drive_addIDEDevice(char* model, u32 size, u8 atapi, u8 present, u8 channel,
                          u8 drv, u16 signature, u16 capabilities, u32 command_sets) {
    
    int name_size = 0;

    for (int i = 0; i < 40; i++)
        if (model[i] != ' ' && model[i] != 0)
            name_size = i + 1;

    char* d_name = kmalloc(name_size + 1);

    for (int i = 0; i < name_size; i++)
        d_name[i] = model[i];

    d_name[name_size] = 0;

    ide_drive* d = kmalloc(sizeof(ide_drive));
    if (d == 0) return 1;

    d->type         = DRIVE_IDE;

    d->device_name  = d_name;
    d->size         = size;

    d->atapi        = atapi;

    d->present      = present;
    d->channel      = channel;
    d->drive        = drv;

    d->signature    = signature;
    d->capabilities = capabilities;
    d->command_sets = command_sets;

    return drive_add((drive*) d);
}

void drive_addPartition(char* name, drive* parent, u32 start_sector, u32 end_sector) {
    drive_partition* p = kmalloc(sizeof(drive_partition));
    if (p == 0) return;

    p->type         = PARTITION;
    p->device_name  = name;
    p->size         = end_sector - start_sector;
    p->parent       = parent;
    p->start_sector = start_sector;
    p->end_sector   = end_sector;
    
    parray_push(&partitions, p);
    
    u8 sector[512];

    if (drive_read(p, 0, 1, sector) == 1)
        return;
}

void drive_init() {
    parray(&drives, 8);
}