#include "allocator.hpp"

#include <cstdlib>

void* cache_alloc(std::size_t size) {
    return std::malloc(size);
}

void cache_free(void* ptr, std::size_t size) {
    (void)size;
    std::free(ptr);
}