#include "iso9660.hpp"

#include "../kmm.hpp"

using namespace apo;

#define LE_BE_U16(N) \
    u16 N; \
    u16 MSB_##N;

#define LE_BE_U32(N) \
    u32 N; \
    u32 MSB_##N;

struct PACKED iso_datetime {
    u8  year;
    u8  month;
    u8  day;
    u8  hour;
    u8  minute;
    u8  second;
    u8  gmt_offset;
};

struct PACKED iso_voldesc {
    u8 type;
    char std_id[5];
    u8 version;
};

struct PACKED iso_dir_entry {
    u8 entry_size;
    u8 ext_attr_size;
    LE_BE_U32(lba);
    LE_BE_U32(size);
    iso_datetime datetime;
    u8 flags;
    u8 file_unit_size;
    u8 interleave_gap;
    LE_BE_U16(volume_seq_num);
    u8 fi_size;
    char name[0];
};

struct PACKED iso_voldesc_primary {
    u8 type;
    char std_id[5];
    u8 version;
    u8 __1;
    char sys_id[32];
    char vol_id[32];
    u8 __2[8];
    LE_BE_U32(vol_space_size);
    u8 __3[32];
    LE_BE_U16(vol_set_size);
    LE_BE_U16(vol_seq_num);
    LE_BE_U16(log_block_size);
    LE_BE_U32(path_table_size);
    u32 path_table_lba;
    u32 op_path_table_lba;
    u32 MSB_path_table_lba;
    u32 MSB_op_path_table_lba;
    iso_dir_entry root_dir;
    u8 __4;
};

struct PACKED iso_pathtab_entry {
    u8 di_size;
    u8 di_ext_size;
    u32 table_lba;
    u16 parent;
    char name[0];
};

struct PACKED iso_sysuse_entry {
    char type[2];
    u8 length;
    u8 version;
};

#define ISO_DFLAG_HIDDEN    0x01
#define ISO_DFLAG_DIR       0x02
#define ISO_DFLAG_ASSOC     0x04
#define ISO_DFLAG_EXT_ATTR  0x08
#define ISO_DFLAG_PERM      0x10
#define ISO_DFLAG_NOT_FINAL 0x80

Iso9660Fs::Iso9660Fs() {}

Iso9660Fs::~Iso9660Fs() {
    for (auto& entry : path_table)
        delete entry.name;

    path_table.~Array();
}

File Iso9660Fs::_getFile(Array<const char*> path) {
    IsoFile file = root;
    
    for (auto& name : path) {
        file = getChild(file, name);
        if (!file.valid)
            return File::invalid();
    }

    File f(this);

    f.is_dir = file.is_dir;
    f.size   = file.size;

    f.data1  = file.lba;
    f.data2  = file.size;

    return f;
}

u32 Iso9660Fs::_read(File f, u32 size, u32 offset, void* buffer) {
    if (!f.valid || f.is_dir)
        return 0;

    if (offset >= f.size)
        return 0;

    size = min(size, f.size - offset);

    u32 file_lba = f.data1;

    u32 begin_sector = offset / 0x200;
    u32 end_sector   = DIVIDE_CEIL(offset + size, 0x200);

    u32 read_lba  = file_lba + begin_sector;
    u32 read_size = end_sector - begin_sector;

    u8* read_buffer;

    read_buffer = new u8[read_size * 0x200];

    if (drive_read(drv, read_lba, read_size, read_buffer) != 0) {
        delete read_buffer;
        return 0;
    }

    memcpy(buffer, read_buffer + (offset % 0x200), size);

    delete read_buffer;
    return size;
}

bool Iso9660Fs::isValid(::drive* drv) {
    if (drv->type == PARTITION)
        return 0;

    u8 buffer[512];

    if (drive_read(drv, 0x10 * 4, 1, buffer) != 0)
        return 0;

    iso_voldesc* vd = (iso_voldesc*)buffer;

    const char* mag = "CD001";

    for (int i = 0; i < 5; i++)
        if (vd->std_id[i] != mag[i])

    if (vd->type != 1 || vd->version != 1)
        return 0;

    return 1;
}

bool isSysUseEntry(const char* e) {
    return e[0] > 64 && e[0] < 91 &&
           e[1] > 64 && e[1] < 91;
}

bool cmpSysUse(const char* e1, const char* e2) {
    return e1[0] == e2[0] && e1[1] == e2[1];
}

IsoDirEntry readDirEntry(iso_dir_entry* entry) {
    IsoDirEntry dir;

    u32 name_len = entry->fi_size;

    u8* sysuse     = (u8*)entry->name + entry->fi_size + (1 - entry->fi_size % 2);
    u32 sysuse_len = entry->entry_size - sizeof(iso_dir_entry) - entry->fi_size - (1 - entry->fi_size % 2);

    char* name = 0;

    int i = 0;
    while (i < sysuse_len) {
        if (!isSysUseEntry((char*)sysuse + i))
            break;

        iso_sysuse_entry* e = (iso_sysuse_entry*)(sysuse + i);

        if (cmpSysUse(e->type, "NM"))
            name = allocString((char*)e + 5, e->length - 5);

        i += e->length;
    }
    
    if (name == 0 && name_len == 0)
        name = "";
    
    if (name == 0 && name_len == 1) {
        if (entry->name[0] == 0)
            name = ".";
        else if (entry->name[0] == 1)
            name = "..";
    }

    if (name == 0) {
        char* name = allocString(entry->name, name_len);

        for (int i = 0; i < name_len; i++)
            if (name[i] == ';')
                name[i] = '\0'; // Maybe at some point we'll parse the file id if i can find out why it's needed
    }

    dir.name = name;

    dir.is_dir = entry->flags & ISO_DFLAG_DIR;
    dir.lba    = entry->lba;
    dir.size   = entry->size;

    return dir;
}

Array<IsoDirEntry> Iso9660Fs::getDirEntries(IsoFile file) {
    u32 sector_count = DIVIDE_CEIL(file.size, 0x200);
    u8* buffer = new u8[sector_count * 0x200];

    if (drive_read(drv, file.lba, sector_count, buffer) != 0) {
        delete buffer;
        return Array<IsoDirEntry>();
    }

    Array<IsoDirEntry> entries;

    int i = 0;
    while (i < file.size) {
        iso_dir_entry* entry = (iso_dir_entry*)(buffer + i);

        if (entry->entry_size == 0) {
            i++;
            continue;
        }

        entries.push(readDirEntry(entry));
        i += entry->entry_size;
    }

    return entries;
}

IsoFile Iso9660Fs::fileFromDirEntry(IsoDirEntry* entry) {
    IsoFile file;

    file.valid  = true;
    file.is_dir = entry->is_dir;
    file.lba    = lbaTo512(entry->lba);
    file.size   = entry->size;
    
    return file;
}

IsoFile Iso9660Fs::getChild(IsoFile dir, const char* name) {
    Array<IsoDirEntry> entries = getDirEntries(dir);
    IsoFile file = {false};

    for (auto& e : entries) {
        if (cmpstr(e.name, name)) {
            file = fileFromDirEntry(&e);
            break;
        }
    }

    for (auto& e : entries)
        delete e.name;

    entries.~Array();
    return file;
}

u32 Iso9660Fs::lbaTo512(u32 lba) {
    return lba * sector_size / 0x200;
}

bool Iso9660Fs::readPathTable() {
    u32 pathtab_sectors = DIVIDE_CEIL(pathtab_size, 0x200);
    u8* pathtab = (u8*)kmalloc(pathtab_sectors * 0x200);

    if (drive_read(drv, pathtab_lba, pathtab_sectors, pathtab) != 0)
        return false;

    int offset = 0, index = 1;
    while (offset < pathtab_size) {
        iso_pathtab_entry* entry = (iso_pathtab_entry*)(pathtab + offset);

        if (entry->di_size == 0)
            continue;

        char* name = allocString(entry->name, entry->di_size);

        IsoPathTableEntry pte;

        pte.parent = entry->parent;
        pte.lba    = lbaTo512(entry->table_lba);
        pte.name   = name;

        path_table.push(pte);

        offset += sizeof(iso_pathtab_entry) + entry->di_size + (entry->di_size % 2);
        index++;
    }

    return true;
}

Filesystem* Iso9660Fs::mount(::drive* drv) {
    if (drv->type == PARTITION)
        return 0;

    u8 buffer[512];

    dbg_write("Mounting ISO9660 filesystem\n");

    if (drive_read(drv, 0x10 * 4, 1, buffer) != 0)
        return 0;

    iso_voldesc* vd = (iso_voldesc*)buffer;

    const char* mag = "CD001";

    for (int i = 0; i < 5; i++)
        if (vd->std_id[i] != mag[i])

    if (vd->type != 1 || vd->version != 1)
        return 0;

    iso_voldesc_primary* pvd = (iso_voldesc_primary*)buffer;

    Iso9660Fs* fs = new Iso9660Fs();

    fs->drv = drv;

    fs->sector_size = pvd->log_block_size;

    fs->pathtab_lba  = fs->lbaTo512(pvd->path_table_lba);
    fs->pathtab_size = pvd->path_table_size;

    if (!fs->readPathTable()) {
        delete fs;
        return nullptr;
    }

    IsoDirEntry root_dir = readDirEntry(&(pvd->root_dir));

    fs->root = fs->fileFromDirEntry(&root_dir);
    root_dir.~IsoDirEntry();

    return fs;
}