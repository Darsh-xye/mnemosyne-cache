#pragma once

#include <string>
#include <unordered_map>

#include "resp.hpp"

class Server {
public:
    Server(int port);
    ~Server();

    void run();

private:
    int port_;
    int server_fd_;
    int epoll_fd_;

    std::unordered_map<int, std::string> client_buffers_;

    RespParser parser_;

    void setup_server();
    void add_client(int client_fd);
    void remove_client(int client_fd);
    void handle_client(int client_fd);
    void process_buffer(int client_fd);
};