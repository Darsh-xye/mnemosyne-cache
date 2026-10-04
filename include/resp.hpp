#pragma once

#include <string>
#include <vector>
#include <cstddef>

struct Command {
    std::string name;
    std::vector<std::string> args;
};

class RespParser {
public:
    bool parse(
        const std::string& buffer,
        Command& command,
        std::size_t& consumed
    );
};