cmake_minimum_required(VERSION 3.15)
project(sccpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(include)

add_executable(sccpp 
    src/main.cpp
    src/logger.cpp
    src/device.cpp
)

target_compile_options(sccpp PRIVATE -Wall -Wextra -g)