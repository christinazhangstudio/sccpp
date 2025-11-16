cmake_minimum_required(VERSION 3.15)
project(sccpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/ffmpeg)
include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)

include_directories(include)

add_executable(sccpp
  src/main.cpp 
  src/logger.cpp
  src/device.cpp
  src/socket.cpp
  src/decoder.cpp
)

target_link_libraries(sccpp
  avcodec
  avutil
  avformat
  swscale
)

if(WIN32)
  target_link_options(sccpp PRIVATE -mconsole)
  target_link_libraries(sccpp Ws2_32)  # link WinSock
endif()

target_compile_options(sccpp PRIVATE -Wall -Wextra -g)