#pragma once

extern "C" {
#include "kmm.h"
}

void* operator new[](long unsigned int s);

void* operator new(long unsigned int s);

void operator delete[](void* p);

void operator delete(void* p, long unsigned int);