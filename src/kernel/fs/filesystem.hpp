#pragma once

extern "C" {
#include "../drv/drive.h"
}

#include "../utils.hpp"

namespace apo {

enum FilesystemType {
    ISO9660
};

class Filesystem;

class File {
public:
    File(Filesystem* fs);

    static File invalid();

    u32 read(void* buffer, u32 size, u32 offset = 0);

public:
    bool valid;
    bool is_dir;
    Filesystem* fs;

    u32 size;

    u32 data1;
    u32 data2;
};

class Filesystem {
    friend class File;
public:
    virtual ~Filesystem();

    File getFile(const char* path);

    drive* getDrive() const;

    static Filesystem* mount(::drive* drv);

    static void setCurrent(Filesystem* fs);

    static Filesystem* current();

protected:
    virtual File _getFile(Array<const char*> path) = 0;
    virtual u32 _read(File file, u32 size, u32 offset, void* buffer) = 0;
    
protected:
    FilesystemType type;

    drive* drv;

private:
    static Filesystem* crnt;
};

}