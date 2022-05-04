#include "filesystem.hpp"

#include "iso9660.hpp"

#include "../kmm.hpp"

using namespace apo;

Filesystem* Filesystem::crnt = nullptr;

File::File(Filesystem* fs)
    : valid(true), fs(fs) {}

File File::invalid() {
    File f(nullptr);
    f.valid = false;
    return f;
}

Filesystem::~Filesystem() {}

u32 File::read(void* buffer, u32 size, u32 offset) {
    return fs->_read(*this, size, offset, buffer);
}

Array<const char*> parsePath(char* file_path) {
    u32 len = strlen(file_path);
    
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

    Array<const char*> array(fncount);

    non_slash = 0;

    for (int i = 0; i < len; i++) {
        char c = file_path[i];
        u8 nsl = c != '/' && c != 0;

        if (!non_slash && nsl)
            array.push(file_path + i);

        non_slash = nsl;
    }
    
    return array;
}

File Filesystem::getFile(const char* path) {
    char* file_path = allocString(path, strlen(path));
    auto fn = parsePath(file_path);

    auto f = _getFile(fn);

    delete file_path;
    fn.~Array();

    return f;
}

drive* Filesystem::getDrive() const {
    return drv;
}

Filesystem* Filesystem::mount(::drive* drv) {
    dbg_write("Mounting filesystem on drive ");
    dbg_write(drive_path(drv));
    dbg_write("\n");

    Filesystem* fs = nullptr;

    if (Iso9660Fs::isValid(drv)) {
        if (fs = Iso9660Fs::mount(drv)) {
            dbg_write("mount: ISO9660 mounted from ");
            dbg_write(drive_path(drv));
            dbg_write("\n");
        }
    }

    return fs;
}

void Filesystem::setCurrent(Filesystem* fs) {
    crnt = fs;
    dbg_write("Current filesystem changed to ");
    dbg_write(drive_path(fs->getDrive()));
    dbg_write("\n");
}

Filesystem* Filesystem::current() {
    return crnt;
}