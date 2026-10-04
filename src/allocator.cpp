#include "allocator.hpp"

#ifdef USE_MNEMOSYNE

#include "allocator.h"

void* cache_alloc(std::size_t size) {
    return ma::alloc(size);
}

void cache_free(void* ptr, std::size_t size) {
    ma::free(ptr, size);
}

#else

#include <cstdlib>

void* cache_alloc(std::size_t size) {
    return std::malloc(size);
}

void cache_free(void* ptr, std::size_t size) {
    (void)size;
    std::free(ptr);
}

#endif