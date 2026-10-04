#pragma once

#include <string>
#include <unordered_map>
#include <cstddef>

struct CacheValue {
    char* data;
    std::size_t size;
};

class Cache {
public:
    Cache() = default;
    ~Cache();

    void set(const std::string& key, const std::string& value);

    bool get(
        const std::string& key,
        std::string& value
    ) const;

    bool del(const std::string& key);

private:
    std::unordered_map<std::string, CacheValue> data_;
};