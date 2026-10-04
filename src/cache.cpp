#include "cache.hpp"

void Cache::set(
    const std::string& key,
    const std::string& value
) {
    data_[key] = value;
}

bool Cache::get(
    const std::string& key,
    std::string& value
) const {
    auto it = data_.find(key);

    if (it == data_.end()) {
        return false;
    }

    value = it->second;
    return true;
}

bool Cache::del(const std::string& key) {
    return data_.erase(key) > 0;
}