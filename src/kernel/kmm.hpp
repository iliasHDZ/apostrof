#pragma once

extern "C" {
#include "kmm.h"
}

void* operator new[](u32 s);

void* operator new(u32 s);

void operator delete[](void* p);

void operator delete(void* p, u32);