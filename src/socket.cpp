#include "socket.h"
#include "logger.h"
#include <cstring>

#ifdef _WIN32
  static bool winsock_initialized = false;
#endif

Socket::Socket() : sock(SOCKET_INVALID) {
#ifdef _WIN32
  // memoize to avoid redundant cleanups
  if(!winsock_initialized) {
    WSADATA wsa;
    // request version 2.2 of Winsock API
    // wsa is used for version and capability info
    // to be written to. returns zero if startup succeeded.
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
      return;
    }
    // with this, socket(), connect(), send(), recv(), closesocket(), ...
    // can be called
    winsock_initialized = true;
  }
#endif
}

Socket::~Socket() {
  close();
}

bool Socket::connect(const char* host, uint16_t port) {
  // AF_INET -> use IPv4 address
  // SOCK_STREAM -> use TCP
  // 0 -> default protocol for TCP
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == SOCKET_INVALID) {
    return false;
  }

  // create and zero-init socket addr structure
  sockaddr_in addr{};

  // fill it with config
  // AF_INET -> IPv4
  // htons() -> "host to network short" in case CPU is little endian
  // (since network protocols always use big-endian, and we want to conform to this)
  // port -> convert port to big endian
  // inet_pton() -> converts IP string to binary addr
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if(inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
    CLOSE_SOCKET(sock);
    sock = SOCKET_INVALID;
    return false;
  }

  // attempt to connect!
  if(::connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0) {
    CLOSE_SOCKET(sock);
    sock = SOCKET_INVALID;
    return false;
  }
  
  return true;
}

bool Socket::send(const uint8_t* data, size_t size, Logger& log) {
  // created from socket(), check if invalid or closed
  if(sock == SOCKET_INVALID) {
    log.error("socket was invalid");
    return false;
  }

  int ret = ::send(sock, (const char*)data, size, 0);
  log.info("sent %d number of bytes", ret);
  
  return ret == (int)size;
}

bool Socket::recv(uint8_t* buffer, size_t size, size_t* received) {
  // created from socket(), check if invalid or closed
  if(sock == SOCKET_INVALID) {
    return false;
  }

  // cast to char* to match POSIX/WinSock API
  // size specified how many bytes to read
  // 0 means no special flags
  // return the number of bytes received,
  // or 0 if the connection was closed gracefully by peer
  // or <0 if error
  int res = ::recv(sock, (char*)buffer, size, 0);
  if(res <= 0) {
    return false;
  }

  // received is an optional arg
  // so we can make the caller aware if the
  // expected number of bytes were recv or not care
  if(received) {
    // deref and assign
    *received = res;
  }
  
  return true;
}

void Socket::close() {
  if(sock != SOCKET_INVALID) {
    CLOSE_SOCKET(sock);
    sock = SOCKET_INVALID;
  }
}
