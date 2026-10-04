#pragma once

#include <unordered_map>
#include <string>

class Server {
public:
    Server(int port);
    ~Server();

    void run();

private:
    int port_;
    int server_fd_;
    int epoll_fd_;

    void setup_server();
    void add_client(int client_fd);
    void handle_client(int client_fd);
};