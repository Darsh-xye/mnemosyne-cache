#pragma once

#include <cstddef>

void* cache_alloc(std::size_t size);
void cache_free(void* ptr, std::size_t size);