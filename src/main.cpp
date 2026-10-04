#include <iostream>
#include "server.hpp"

int main() {
    std::cout << "Mnemosyne-Cache starting...\n";

    Server server;
    server.run();

    return 0;
}