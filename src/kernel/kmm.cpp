#include "kmm.hpp"

void* operator new[](u32 s) {
    return kmalloc(s);
}

void* operator new(u32 s) {
    return kmalloc(s);
}

void operator delete[](void* p) {
    return kfree(p);
}

void operator delete(void* p, u32 s) {
    return kfree(p);
}