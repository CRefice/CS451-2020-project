#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <optional>

namespace udp {
sockaddr_in socket_address(in_addr_t ip, std::uint16_t port);

class Socket {
public:
  Socket(std::uint16_t port);
  Socket(const Socket&) = delete;
  Socket(Socket&&) = default;
  ~Socket();

  Socket& operator=(const Socket&) = delete;
  Socket& operator=(Socket&&) = default;

  void send(const sockaddr_in& dst_addr, const char* buf, std::size_t len);

  template <typename T>
  void send(const sockaddr_in& dst_addr, const T& val) {
    send(dst_addr, reinterpret_cast<const char*>(&val), sizeof(T));
  }

  std::optional<std::size_t> try_recv(char* buf, std::size_t len);
  std::size_t recv(char* buf, std::size_t len);

private:
  int fd = 0;
};
} // namespace udp
