#include "cache.hpp"
#include "allocator.hpp"

#include <cstring>

Cache::~Cache() {
    for (auto& [key, value] : data_) {
        cache_free(value.data, value.size);
    }
}

void Cache::set(
    const std::string& key,
    const std::string& value
) {
    char* new_data = static_cast<char*>(
        cache_alloc(value.size())
    );

    if (new_data == nullptr) {
        return;
    }

    std::memcpy(
        new_data,
        value.data(),
        value.size()
    );

    auto it = data_.find(key);

    if (it != data_.end()) {
        cache_free(
            it->second.data,
            it->second.size
        );

        it->second.data = new_data;
        it->second.size = value.size();

        return;
    }

    data_[key] = {
        new_data,
        value.size()
    };
}

bool Cache::get(
    const std::string& key,
    std::string& value
) const {
    auto it = data_.find(key);

    if (it == data_.end()) {
        return false;
    }

    value.assign(
        it->second.data,
        it->second.size
    );

    return true;
}

bool Cache::del(const std::string& key) {
    auto it = data_.find(key);

    if (it == data_.end()) {
        return false;
    }

    cache_free(
        it->second.data,
        it->second.size
    );

    data_.erase(it);

    return true;
}