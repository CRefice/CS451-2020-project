#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "udp.hpp"

namespace udp {
sockaddr_in socket_address(in_addr_t ip, std::uint16_t port) {
  sockaddr_in addr{};
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // IPv4
  addr.sin_port = port;
  addr.sin_addr.s_addr = ip;
  return addr;
}

Socket::Socket(std::uint16_t port) {
  // Create a socket file descriptor
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw std::runtime_error(std::string("socket creation failed: ") +
                             std::strerror(errno));
  }
  // Fill in socket address information
  sockaddr_in bind_addr{};
  std::memset(&bind_addr, 0, sizeof(bind_addr));
  bind_addr.sin_family = AF_INET; // IPv4
  bind_addr.sin_addr.s_addr = INADDR_ANY;
  bind_addr.sin_port = port;
  // Bind the socket with the address
  if (bind(fd, reinterpret_cast<const sockaddr*>(&bind_addr),
           sizeof(bind_addr)) < 0) {
    throw std::runtime_error(std::string("socket bind failed: ") +
                             std::strerror(errno));
  }
}

Socket::~Socket() { close(fd); }

void Socket::send(const sockaddr_in& dst_addr, const char* buf,
                  std::size_t len) {
  if (sendto(fd, buf, len, 0, reinterpret_cast<const sockaddr*>(&dst_addr),
             sizeof(dst_addr)) < 0) {
    throw std::runtime_error(std::string("failed to send message: ") +
                             std::strerror(errno));
  }
}

std::optional<sockaddr_in> Socket::try_recv(char* buf, std::size_t len) {
  // Do not block and wait for a message if it's not available,
  // instead simply return std::nullopt.
  static constexpr int flags = MSG_DONTWAIT;
  sockaddr_in src_addr{};
  socklen_t addr_len = sizeof(src_addr);
  if (recvfrom(fd, buf, len, flags, reinterpret_cast<sockaddr*>(&src_addr),
               &addr_len) < 0) {
    if (errno == EWOULDBLOCK) {
      return std::nullopt;
    }
    throw std::runtime_error(std::string("failed to receive message: ") +
                             std::strerror(errno));
  }
  return src_addr;
}

sockaddr_in Socket::recv(char* buf, std::size_t len) {
  sockaddr_in src_addr{};
  socklen_t addr_len = sizeof(src_addr);
  if (recvfrom(fd, buf, len, 0, reinterpret_cast<sockaddr*>(&src_addr),
               &addr_len) < 0) {
    throw std::runtime_error(std::string("failed to receive message: ") +
                             std::strerror(errno));
  }
  return src_addr;
}
} // namespace udp
