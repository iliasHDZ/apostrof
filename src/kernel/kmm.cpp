#include "kmm.hpp"

void* operator new[](long unsigned int s) {
    return kmalloc(s);
}

void* operator new(long unsigned int s) {
    return kmalloc(s);
}

void operator delete[](void* p) {
    return kfree(p);
}

void operator delete(void* p, long unsigned int s) {
    return kfree(p);
}