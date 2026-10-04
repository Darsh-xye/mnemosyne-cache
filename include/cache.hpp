#pragma once

#include <string>
#include <unordered_map>

class Cache {
public:
    void set(const std::string& key, const std::string& value);

    bool get(
        const std::string& key,
        std::string& value
    ) const;

    bool del(const std::string& key);

private:
    std::unordered_map<std::string, std::string> data_;
};