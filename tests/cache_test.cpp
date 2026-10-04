#include "cache.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    Cache cache;

    std::string value;

    cache.set("name", "darsh");

    assert(cache.get("name", value));
    assert(value == "darsh");

    cache.set("name", "mnemosyne");

    assert(cache.get("name", value));
    assert(value == "mnemosyne");

    assert(cache.del("name"));

    assert(!cache.get("name", value));

    assert(!cache.del("missing"));

    std::cout << "cache tests passed\n";

    return 0;
}