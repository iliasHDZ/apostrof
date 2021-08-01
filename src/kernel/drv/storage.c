#include "storage.h"

#include "../kmm.h"
#include "ide.h"

storage_dev** storage_devs;
int           storage_dev_max;

int           storage_dev_count;

storage_dev* storage_getDevice(int index) {
    if (index < 0 || index >= storage_dev_count) return 0;
    return storage_devs[index];
}

char storage_path_str[20] = "/dev/";

const char* storage_path(storage_dev* d) {
    if (d == 0) return "unknown";

    if (d->type == STORAGE_IDE) {
        ide_drive* ide = (ide_drive*)d;

        memcpy(storage_path_str + 5, "hdx", 4);
        storage_path_str[7] = ("abcd")[ide->channel << 1 | ide->drive];

        return storage_path_str;
    } else return "unknown";
}

u8 storage_read(storage_dev* d, u32 sector, u8 sectorcount, u8* dst) {
    if (d->type == STORAGE_IDE) {
        ide_drive* ide = (ide_drive*) d;

        if (!ide->atapi)
            return ide_ata_read(ide, sector, sectorcount, dst);
        else return 1;
    }
}

u8 storage_add(storage_dev* d) {
    if (storage_dev_count >= storage_dev_max) {
        storage_dev_max  += 5;
        storage_dev** new_devs = krealloc(storage_devs, storage_dev_max);

        if (new_devs != 0) storage_devs = new_devs;
        else return 1;
    }

    storage_devs[storage_dev_count++] = d;
}

u8 storage_addIDEDevice(char* model, u32 size, u8 atapi, u8 present, u8 channel,
                          u8 drive, u16 signature, u16 capabilities, u32 command_sets) {
    
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

    d->type         = STORAGE_IDE;

    d->device_name  = d_name;
    d->size         = size;

    d->atapi        = atapi;

    d->present      = present;
    d->channel      = channel;
    d->drive        = drive;

    d->signature    = signature;
    d->capabilities = capabilities;
    d->command_sets = command_sets;

    return storage_add((storage_dev*) d);
}

void storage_init() {
    storage_dev_count = 0;
    storage_dev_max   = 10;

    storage_devs = kmalloc(sizeof(storage_dev**) * storage_dev_max);
}