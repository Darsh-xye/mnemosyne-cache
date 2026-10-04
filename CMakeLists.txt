cmake_minimum_required(VERSION 3.16)

project(mnemosyne-cache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mnemosyne-cache
    src/main.cpp
    src/server.cpp
    src/resp.cpp
)

target_include_directories(mnemosyne-cache PRIVATE include)