#include "apofs.h"

#include "../kmm.h"
#include "../error.h"
#include "../vga.h"

#define APOFS_STD_DIR  0x01
#define APOFS_STD_FILE 0x02

#define APOFS_CHILDREN_DATA 0x58

apo_fs* current_root = 0;

typedef struct {
    u16 jump;
    u8  fs_name[8];
    u32 disk_type;
    u32 bitmap_base;
    u32 table_base;
    u32 file_base;
    u16 desc_size;
} __attribute__((packed)) apofs_header;

typedef struct {
    u8   type;
    u32  parent;
    char name[59];
    u64  creation_date;
    u64  modified_date;
    u16  attributes;
    u8   padding[6];
} __attribute__((packed)) apofs_std_filedesc;

u16 apofs_last_error = 0;

u8 cmpstr_len(char* str1, char* str2, int len) {
    for (int i = 0; i < len; i++)
        if (str1[i] != str2[i]) return 0;

    return 1;
}

u8 cmpstr(const char* str1, const char* str2) {
    int i = 0;

    while(str1[i] != 0 && str2[i] != 0) {
        if (str1[i] != str2[i]) return 0;
        i++;    
    }

    if (str1[i] != str2[i]) return 0;
    return 1;
}

u16 apofs_lastError() {
    return apofs_last_error;
}

#define APOFS_READ_FAIL             0x0001
#define APOFS_INVALID_FS_TEXT       0x0002
#define APOFS_TABLE_BASE_TOO_FAR    0x0003
#define APOFS_BITMAP_BASE_TOO_FAR   0x0004
#define APOFS_FILE_BASE_OUT_OF_DISK 0x0005
#define APOFS_DESC_SIZE_TOO_BIG     0x0006
#define APOFS_NOT_A_FILE            0x0007
#define APOFS_FILE_NOT_PRESENT      0x0008
#define APOFS_NOT_A_DIR             0x0009

apo_fs* apofs_openDevice(storage_dev* d) {
    if (d == 0) return 0;
    if (d->size < 1) return 0;

    apofs_header* bootsec = kmalloc(512);

    if (storage_read(d, 0, 1, (u8*)bootsec) != 0) {
        kfree(bootsec);
        apofs_last_error = APOFS_READ_FAIL;
        return 0;
    }

    if (!cmpstr_len(bootsec->fs_name, "APOFS   ", 8)) {
        kfree(bootsec);
        apofs_last_error = APOFS_INVALID_FS_TEXT;
        return 0;
    }

    if (bootsec->file_base < bootsec->table_base) {
        kfree(bootsec);
        apofs_last_error = APOFS_TABLE_BASE_TOO_FAR;
        return 0;
    }

    if (bootsec->table_base < bootsec->bitmap_base) {
        kfree(bootsec);
        apofs_last_error = APOFS_BITMAP_BASE_TOO_FAR;
        return 0;
    }

    if (bootsec->file_base >= d->size) {
        kfree(bootsec);
        apofs_last_error = APOFS_FILE_BASE_OUT_OF_DISK;
        return 0;
    }

    if (bootsec->desc_size > 8) {
        kfree(bootsec);
        apofs_last_error = APOFS_DESC_SIZE_TOO_BIG;
        return 0;
    }

    u32 bitmap_size = bootsec->table_base - bootsec->bitmap_base;
    u8* bitmap      = kmalloc(bitmap_size);

    if (storage_read(d, bootsec->bitmap_base, bitmap_size, bitmap) != 0) {
        kfree(bootsec);
        kfree(bitmap);
        apofs_last_error = APOFS_READ_FAIL;
        return 0;
    }

    apo_fs* fs = kmalloc(sizeof(apo_fs));

    fs->device = d;

    fs->bitmap_base = bootsec->bitmap_base;
    fs->table_base  = bootsec->table_base;
    fs->file_base   = bootsec->file_base;
    fs->desc_size   = bootsec->desc_size;
    
    fs->bitmap      = bitmap;
    fs->bitmap_size = bitmap_size;

    kfree(bootsec);
    return fs;
}

u8 apofs_isPresent(apo_fs* fs, u32 file_id) {
    if (file_id == 0) return 0;
    if (((file_id - 1) * fs->desc_size) >= (fs->file_base - fs->table_base)) return 0;

    if ((file_id - 1) >= fs->bitmap_size * 512 * 8) return 0;

    u32 byte = (file_id - 1) / 8;
    u8  bit  = (file_id - 1) % 8;

    return (fs->bitmap[byte] >> bit) & 0x01;
}

u8 apofs_setPresent(apo_fs* fs, u32 file_id, u8 present) {
    if (file_id == 0) return 1;
    if (((file_id - 1) * fs->desc_size) >= (fs->file_base - fs->table_base)) return 1;
    if ((file_id - 1) >= fs->bitmap_size * 512 * 8) return 1;
    u32 byte = (file_id - 1) / 8;
    u8  bit  = (file_id - 1) % 8;

    if (present) fs->bitmap[byte] |=   1 << bit;
    else         fs->bitmap[byte] &= ~(1 << bit);
    return 0;
}

u8 apofs_getFileName(apo_fs* fs, u32 file_id, char* name_out, int len_out) {
    if (!apofs_isPresent(fs, file_id)) {
        apofs_last_error = APOFS_FILE_NOT_PRESENT;
        return 1;
    }

    int sector = (file_id - 1) * fs->desc_size + fs->table_base;
    
    apofs_std_filedesc* file = kmalloc(512);

    if (storage_read(fs->device, sector, 1, (u8*)file) != 0) {
        kfree(file);
        apofs_last_error = APOFS_READ_FAIL;
        return 1;
    }

    if (file->type != APOFS_STD_DIR && file->type != APOFS_STD_FILE)  {
        kfree(file);
        apofs_last_error = APOFS_NOT_A_FILE;
        return 1;
    }

    int i = 0;
    for (i = 0; i < len_out && i < 59 && file->name[i] != 0; i++)
        name_out[i] = file->name[i];
    
    name_out[i] = 0;

    kfree(file);
    return 0;
}

u8 apofs_getFileInfo(apo_fs* fs, u32 file_id, apofs_fileinfo* info_out) {
    if (!apofs_isPresent(fs, file_id)) {
        apofs_last_error = APOFS_FILE_NOT_PRESENT;
        return 1;
    }

    int sector = (file_id - 1) * fs->desc_size + fs->table_base;
    
    apofs_std_filedesc* file = kmalloc(512);

    if (storage_read(fs->device, sector, 1, (u8*)file) != 0) {
        kfree(file);
        apofs_last_error = APOFS_READ_FAIL;
        return 1;
    }

    if (file->type != APOFS_STD_DIR && file->type != APOFS_STD_FILE)  {
        kfree(file);
        apofs_last_error = APOFS_NOT_A_FILE;
        return 1;
    }

    info_out->dir           = file->type == APOFS_STD_DIR;
    info_out->parent        = file->parent;
    info_out->creation_date = file->creation_date;
    info_out->modified_date = file->modified_date;
    info_out->attributes    = file->attributes;

    kfree(file);
    return 0;
}

u32 apofs_getChild(apo_fs* fs, u32 file_id, const char* child_name) {
    if (!apofs_isPresent(fs, file_id)) {
        apofs_last_error = APOFS_FILE_NOT_PRESENT;
        return 0;
    }

    u8 file[512];

    for (int s = 0; s < fs->desc_size; s++) {
        int sector = (file_id - 1) * fs->desc_size + fs->table_base + s;

        if (storage_read(fs->device, sector, 1, file) != 0) {
            apofs_last_error = APOFS_READ_FAIL;
            return 0;
        }

        if (file[0] != APOFS_STD_DIR && s == 0) {
            apofs_last_error = APOFS_NOT_A_DIR;
            return 0;
        }

        u32* children   = (u32*)(file + ((u8*)s == 0 ? APOFS_CHILDREN_DATA : 0));
        int  childCount = (512 - ((u8*)s == 0 ? APOFS_CHILDREN_DATA : 0)) / sizeof(u32);

        char file_name[60];
        file_name[59] = 0;

        for (int i = 0; i < childCount; i++) {
            if (children[i] == 0) {
                apofs_last_error = 0;
                return 0;
            }

            if (apofs_getFileName(fs, children[i], file_name, 59) != 0)
                continue;

            if (cmpstr(child_name, file_name)) {
                return children[i];
            }
        }
    }
    
    apofs_last_error = 0;
    return 0;
}

u32 apofs_getFile(apo_fs* fs, const char* path) {
    int len = 0;
    while (path[len] != 0)
        len++;

    char* file_path = kmalloc(len + 1);
    if (!file_path) return 0;

    memcpy(file_path, path, len);
    file_path[len] = 0;
    
    u8 non_slash = 0;

    int fncount = 0;

    for (int i = 0; i < len; i++) {
        char c = file_path[i];

        if (non_slash && (c == '/')) {
            fncount++;
            file_path[i] = 0;
        }

        non_slash = c != '/';
    }

    if (non_slash) fncount++;

    char** fns = kmalloc(sizeof(char*) * fncount);
    if (!fns) return 0;

    non_slash = 0;
    int fn = 0;

    for (int i = 0; i < len; i++) {
        char c = file_path[i];
        u8 nsl = c != '/' && c != 0;

        if (!non_slash && nsl) {
            fns[fn] = file_path + i;
            fn++;
        }

        non_slash = nsl;
    }

    u32 file_id = 1;

    for (int i = 0; i < fncount; i++) {
        file_id = apofs_getChild(fs, file_id, fns[i]);

        if (file_id == 0) {
            kfree(file_path);
            kfree(fns);
            apofs_last_error = 0;
            return 0;
        }
    }

    kfree(file_path);
    kfree(fns);
    return file_id;
}

u32 apofs_getFileSize(apo_fs* fs, u32 file_id) {
    if (!apofs_isPresent(fs, file_id)) {
        apofs_last_error = APOFS_FILE_NOT_PRESENT;
        return 0;
    }

    u8 file[512];

    u32 size = 0;

    for (int s = 0; s < fs->desc_size; s++) {
        int sector = (file_id - 1) * fs->desc_size + fs->table_base + s;

        if (storage_read(fs->device, sector, 1, file) != 0) {
            apofs_last_error = APOFS_READ_FAIL;
            return 0;
        }

        if (file[0] != APOFS_STD_FILE && s == 0) {
            apofs_last_error = APOFS_NOT_A_DIR;
            return 0;
        }

        u32 header_offset = (u8*)s == 0 ? APOFS_CHILDREN_DATA : 0;

        u32* children   = (u32*)(file + header_offset);
        int  childCount = (512 - header_offset) / sizeof(u32);

        for (int i = 0; i < childCount; i += 2) {
            if (children[i] == 0) 
                return size * 512 - 512 + children[i + 1];

            u32 base  = children[i];
            u32 limit = children[i + 1];

            size += limit - base;
        }
    }

    return size * 512;
}

u32 limit_value(u32 value, u32 min, u32 max) {
    return value > max ? max : (value < min ? min : value);
}

u32 apofs_read(apo_fs* fs, u32 file_id, u8* dst, u32 offset, u32 count) {
    if (!apofs_isPresent(fs, file_id)) {
        apofs_last_error = APOFS_FILE_NOT_PRESENT;
        return 0;
    }

    if (count == 0) {
        apofs_last_error = 0;
        return 0;
    }

    u32 base_sector  = offset / 512;
    u32 limit_sector = (offset + count) / 512 + 1;

    u8* file = kmalloc(fs->desc_size * 512);
    u8 content_sector[512];

    int sector = (file_id - 1) * fs->desc_size + fs->table_base;

    if (storage_read(fs->device, sector, fs->desc_size, file) != 0) {
        apofs_last_error = APOFS_READ_FAIL;
        return 0;
    }

    if (file[0] == APOFS_STD_DIR) {
        apofs_last_error = APOFS_NOT_A_FILE;
        return 0;
    }

    u32 bytes_read    = 0;

    u32* children     = (u32*)(file + APOFS_CHILDREN_DATA);
    int  childCount   = (fs->desc_size * 512 - APOFS_CHILDREN_DATA) / sizeof(u32);

    u32 sector_offset = 0;
    u32 dst_offset    = 0;

    for (int i = 0; i < childCount; i += 2) {
        if (children[i] == 0) {
            kfree(file);
            apofs_last_error = 0;
            return dst_offset;
        }

        u32 base  = children[i];
        u32 limit = children[i + 1];

        u32 last_size = 512;

        if (i != childCount - 1 && children[i + 2] == 0)
            last_size = children[i + 3];

        u32 sector_count = limit - base;

        u32 read_base  = limit_value(base_sector  - sector_offset, 0, sector_count);
        u32 read_limit = limit_value(limit_sector - sector_offset, 0, sector_count);

        for (u32 j = read_base; j < read_limit; j++) {
            u32 read_begin, read_end;

            if (storage_read(fs->device, base + j, 1, content_sector) != 0) {
                apofs_last_error = APOFS_READ_FAIL;
                return 0;
            }

            if (sector_offset + j == base_sector)
                read_begin = offset % 512;
            else
                read_begin = 0;

            if (sector_offset + j == (limit_sector - 1))
                read_end = (offset + count) % 512;
            else
                read_end = 512;

            if (j == sector_count - 1 && read_end > last_size)
                read_end = last_size;

            memcpy(dst + dst_offset, content_sector + read_begin, read_end - read_begin);
            dst_offset += read_end - read_begin;
        }

        sector_offset += sector_count;
    }
    
    kfree(file);
    apofs_last_error = 0;
    return dst_offset;
}

void apofs_init() {
    u8 sector_buffer[512];

    u32 current_disk_type = *(u32*)0x7C0A;

    storage_dev* d;
    for (int i = 0; (d = storage_getDevice(i)) != 0; i++) {
        if (d->size < 1) continue;
        if (storage_read(d, 0, 1, sector_buffer) != 0) continue;
        if (*(u32*)(sector_buffer + 0x0A) != current_disk_type) continue;

        current_root = apofs_openDevice(d);
        if (current_root == 0) continue;
    }

    if (current_root == 0) error("Cannot find the current boot medium."
                                 " Please ensure that the boot medium is a CDROM inserted into an IDE drive.");
}