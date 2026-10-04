#include "server.hpp"

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

Server::Server(int port)
    : port_(port), server_fd_(-1) {
}

Server::~Server() {
    if (server_fd_ != -1) {
        close(server_fd_);
    }
}

void Server::run() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd_ == -1) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    int opt = 1;

    setsockopt(
        server_fd_,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(
            server_fd_,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) == -1) {

        std::cerr << "Bind failed\n";
        return;
    }

    if (listen(server_fd_, 128) == -1) {
        std::cerr << "Listen failed\n";
        return;
    }

    std::cout << "Listening on port " << port_ << "\n";

    while (true) {
        int client_fd = accept(server_fd_, nullptr, nullptr);

        if (client_fd == -1) {
            std::cerr << "Accept failed\n";
            continue;
        }

        char buffer[1024];

        ssize_t bytes_read = read(
            client_fd,
            buffer,
            sizeof(buffer) - 1
        );

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            const char* response = "PONG\n";

            write(
                client_fd,
                response,
                std::strlen(response)
            );
        }

        close(client_fd);
    }
}