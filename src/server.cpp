#include "server.hpp"

#include <iostream>
#include <cstring>
#include <cerrno>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

Server::Server(int port)
    : port_(port), server_fd_(-1), epoll_fd_(-1) {
}

Server::~Server() {
    for (auto& [fd, buffer] : client_buffers_) {
        close(fd);
    }

    if (server_fd_ != -1) {
        close(server_fd_);
    }

    if (epoll_fd_ != -1) {
        close(epoll_fd_);
    }
}

void Server::setup_server() {
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

    int flags = fcntl(server_fd_, F_GETFL, 0);
    fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);

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

    epoll_fd_ = epoll_create1(0);

    if (epoll_fd_ == -1) {
        std::cerr << "Failed to create epoll instance\n";
        return;
    }

    epoll_event event{};

    event.events = EPOLLIN;
    event.data.fd = server_fd_;

    epoll_ctl(
        epoll_fd_,
        EPOLL_CTL_ADD,
        server_fd_,
        &event
    );
}

void Server::add_client(int client_fd) {
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    epoll_event event{};

    event.events = EPOLLIN;
    event.data.fd = client_fd;

    epoll_ctl(
        epoll_fd_,
        EPOLL_CTL_ADD,
        client_fd,
        &event
    );

    client_buffers_[client_fd] = "";
}

void Server::remove_client(int client_fd) {
    epoll_ctl(
        epoll_fd_,
        EPOLL_CTL_DEL,
        client_fd,
        nullptr
    );

    client_buffers_.erase(client_fd);

    close(client_fd);
}

void Server::process_buffer(int client_fd) {
    std::string& buffer = client_buffers_[client_fd];

    while (true) {
        Command command;
        std::size_t consumed = 0;

        if (!parser_.parse(
                buffer,
                command,
                consumed
            )) {
            break;
        }

        buffer.erase(0, consumed);

        if (command.name == "PING") {
            const char* response = "+PONG\r\n";

            write(
                client_fd,
                response,
                std::strlen(response)
            );
        }

        else if (command.name == "SET") {
            if (command.args.size() != 2) {
                const char* response =
                    "-ERR wrong number of arguments\r\n";

                write(
                    client_fd,
                    response,
                    std::strlen(response)
                );

                continue;
            }

            cache_.set(
                command.args[0],
                command.args[1]
            );

            const char* response = "+OK\r\n";

            write(
                client_fd,
                response,
                std::strlen(response)
            );
        }

        else if (command.name == "GET") {
            if (command.args.size() != 1) {
                const char* response =
                    "-ERR wrong number of arguments\r\n";

                write(
                    client_fd,
                    response,
                    std::strlen(response)
                );

                continue;
            }

            std::string value;

            if (cache_.get(command.args[0], value)) {

                std::string response =
                    "$" +
                    std::to_string(value.size()) +
                    "\r\n" +
                    value +
                    "\r\n";

                write(
                    client_fd,
                    response.c_str(),
                    response.size()
                );

            } else {
                const char* response = "$-1\r\n";

                write(
                    client_fd,
                    response,
                    std::strlen(response)
                );
            }
        }

        else if (command.name == "DEL") {
            if (command.args.size() != 1) {
                const char* response =
                    "-ERR wrong number of arguments\r\n";

                write(
                    client_fd,
                    response,
                    std::strlen(response)
                );

                continue;
            }

            bool deleted = cache_.del(
                command.args[0]
            );

            std::string response =
                ":" +
                std::to_string(deleted ? 1 : 0) +
                "\r\n";

            write(
                client_fd,
                response.c_str(),
                response.size()
            );
        }

        else {
            const char* response =
                "-ERR unknown command\r\n";

            write(
                client_fd,
                response,
                std::strlen(response)
            );
        }
    }
}

void Server::handle_client(int client_fd) {
    char buffer[4096];

    while (true) {
        ssize_t bytes_read = read(
            client_fd,
            buffer,
            sizeof(buffer)
        );

        if (bytes_read > 0) {
            client_buffers_[client_fd].append(
                buffer,
                bytes_read
            );

            continue;
        }

        if (bytes_read == 0) {
            remove_client(client_fd);
            return;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        }

        remove_client(client_fd);
        return;
    }

    process_buffer(client_fd);
}

void Server::run() {
    setup_server();

    if (server_fd_ == -1 || epoll_fd_ == -1) {
        return;
    }

    std::cout << "Listening on port " << port_ << "\n";

    constexpr int MAX_EVENTS = 64;

    epoll_event events[MAX_EVENTS];

    while (true) {
        int event_count = epoll_wait(
            epoll_fd_,
            events,
            MAX_EVENTS,
            -1
        );

        if (event_count == -1) {
            continue;
        }

        for (int i = 0; i < event_count; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd_) {
                while (true) {
                    int client_fd = accept(
                        server_fd_,
                        nullptr,
                        nullptr
                    );

                    if (client_fd == -1) {
                        break;
                    }

                    add_client(client_fd);
                }
            } else {
                handle_client(fd);
            }
        }
    }
}