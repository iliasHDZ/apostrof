#pragma once

#include "filesystem.hpp"
#include "../utils.hpp"

namespace apo {

struct IsoPathTableEntry {
    u16 parent;
    u32 lba;
    char* name;
};

struct IsoDirEntry {
    const char* name;
    bool is_dir;
    u32 lba;
    u32 size;
};

struct IsoFile {
    bool valid;
    bool is_dir;
    u32 lba;
    u32 size;
};

class Iso9660Fs : public Filesystem {
    friend class Filesystem;
    friend class File;

public:
    Iso9660Fs();
    ~Iso9660Fs();

    static Filesystem* mount(::drive* drv);

protected:
    File _getFile(Array<const char*> path) override;
    u32 _read(File file, u32 size, u32 offset, void* buffer) override;

private:
    static bool isValid(::drive* drv);

    Array<IsoDirEntry> getDirEntries(IsoFile file);

    IsoFile fileFromDirEntry(IsoDirEntry* entry);

    IsoFile getChild(IsoFile dir, const char* name);

    u32 lbaTo512(u32 lba);

    bool readPathTable();

private:
    u32 pathtab_lba;
    u32 pathtab_size;

    u32 sector_size;

    IsoFile root;

    Array<IsoPathTableEntry> path_table;
};

}