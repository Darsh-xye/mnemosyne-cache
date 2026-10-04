#include "resp.hpp"

#include <string>

bool RespParser::parse(
    const std::string& buffer,
    Command& command,
    std::size_t& consumed
) {
    consumed = 0;

    if (buffer.empty()) {
        return false;
    }

    if (buffer[0] != '*') {
        return false;
    }

    std::size_t pos = buffer.find("\r\n");

    if (pos == std::string::npos) {
        return false;
    }

    int element_count = std::stoi(
        buffer.substr(1, pos - 1)
    );

    std::size_t current = pos + 2;

    std::vector<std::string> elements;

    for (int i = 0; i < element_count; i++) {

        if (current >= buffer.size()) {
            return false;
        }

        if (buffer[current] != '$') {
            return false;
        }

        std::size_t length_end = buffer.find(
            "\r\n",
            current
        );

        if (length_end == std::string::npos) {
            return false;
        }

        int length = std::stoi(
            buffer.substr(
                current + 1,
                length_end - current - 1
            )
        );

        std::size_t data_start = length_end + 2;

        if (data_start + length + 2 > buffer.size()) {
            return false;
        }

        std::string value = buffer.substr(
            data_start,
            length
        );

        elements.push_back(value);

        current = data_start + length + 2;
    }

    if (elements.empty()) {
        return false;
    }

    command.name = elements[0];

    command.args.assign(
        elements.begin() + 1,
        elements.end()
    );

    consumed = current;

    return true;
}