#include "utils.hpp"
#include "kmm.hpp"

using namespace apo;

char* apo::allocString(const char* str, u32 len) {
    char* buffer = new char[len + 1];
    memcpy(buffer, str, len);
    buffer[len] = 0;
    
    return buffer;
}