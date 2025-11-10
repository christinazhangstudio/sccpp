#pragma once
#include <cstddef>
#include <cstdint>
#include "logger.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  // link Ws2_32.lib, a Winsock 2 networking library.
  // just an alternative to specifying lib in external deps
  // in project settings (and mostly used to support diff versions)
  #pragma comment(lib, "Ws2_32.lib")
  // aliases for some types from _SOCKET_TYPES_H
  using socket_t = SOCKET;
  #define SOCKET_INVALID INVALID_SOCKET
  #define CLOSE_SOCKET(s) closesocket(s)
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  using socket_t = int;
  #define SOCKET_INVALID (-1)
  #define CLOSE_SOCKET(s) close(s)
#endif

class Socket {
private:
  socket_t sock;

public:
  Socket();
  ~Socket();

  // unint8_t may not be very useful in other contexts,
  // and you could just use `unsigned char` (one byte),
  // (and this is how it's defined in cstdint anyway)
  // but for sockets, POSIX/Windows defines byte steam
  // arguments as void* pointer to a memory region and a byte len
  // we can make it more type safe by using unint8_t*
  // to avoid any accidental implicit casts to void*
  // char* can be cased to unint8_t* on the other hand.
  bool connect(const char* host, uint16_t port);
  bool send(const uint8_t* data, size_t size, Logger& log);
  bool recv(uint8_t* buffer, size_t size, size_t* received = nullptr);
  void close();
};