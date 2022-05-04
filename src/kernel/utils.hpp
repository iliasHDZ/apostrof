#pragma once

extern "C" {
#include "utils.h"
#include "kmm.h"
}

namespace apo {

char* allocString(const char* str, u32 len);


template <typename T>
inline T min(T a, T b) {
    return a < b ? a : b;
}

template <typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template <typename T>
class Array {
public:
    Array() : count(0), capacity(10) {
        buffer = new T[capacity * sizeof(T)];
    }

    Array(u32 cap) : count(0), capacity(cap) {
        buffer = new T[capacity * sizeof(T)];
    }

    ~Array() {
        delete buffer;
    }

    void push(T t) {
        if (count == capacity) {
            capacity += 10;
            buffer = (T*)krealloc(buffer, capacity * sizeof(T));
        }

        buffer[count++] = t;
    }

    T& at(u32 i) const {
        return buffer[i];
    }

    T& operator[](u32 i) const {
        return buffer[i];
    }

    u32 size() const {
        return count;
    }

    T* begin() const {
        return buffer;
    }

    T* end() const {
        return buffer + count;
    }

private:
    T* buffer;

    u32 count;
    u32 capacity;
};

}