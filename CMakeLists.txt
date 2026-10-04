cmake_minimum_required(VERSION 3.16)

project(mnemosyne-cache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_MNEMOSYNE "Use Mnemosyne allocator" OFF)

add_executable(mnemosyne-cache
    src/main.cpp
    src/server.cpp
    src/resp.cpp
    src/cache.cpp
    src/allocator.cpp
)

target_include_directories(
    mnemosyne-cache
    PRIVATE
    include
)

if(USE_MNEMOSYNE)
    add_subdirectory(
        ../mnemosyne
        ${CMAKE_BINARY_DIR}/mnemosyne
    )

    target_link_libraries(
        mnemosyne-cache
        PRIVATE
        mnemosyne
    )

    target_compile_definitions(
        mnemosyne-cache
        PRIVATE
        USE_MNEMOSYNE
    )
endif()