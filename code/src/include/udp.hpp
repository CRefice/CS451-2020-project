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

  std::optional<sockaddr_in> try_recv(char* buf, std::size_t len);
  sockaddr_in recv(char* buf, std::size_t len);

private:
  int fd = 0;
};
} // namespace udp
