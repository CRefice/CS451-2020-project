#include <chrono>
#include <iostream>
#include <thread>

#include <csignal>

#include "barrier.hpp"
#include "broadcast.hpp"
#include "logger.hpp"
#include "parser.hpp"

static std::uint16_t local_port(Parser& parser) {
  for (auto& host : parser.hosts()) {
    if (host.id == parser.id()) {
      return host.port;
    }
  }
  throw std::runtime_error("no host with the given id found in the hosts file");
}

// Ugly have to have global access to a logger in a function
static Logger& logger(const char* filename) {
  static Logger ret(filename);
  return ret;
}

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";

  // write/flush output file if necessary
  std::cout << "Writing output.\n";
  logger(nullptr).flush();

  // exit directly from signal handler
  exit(0);
}

int main(int argc, char** argv) {
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;

  Parser parser(argc, argv, requireConfig);
  parser.parse();

  auto id = parser.id();
  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "My ID: " << id << "\n\n";

  auto hosts = parser.hosts();
  auto barrier = parser.barrier();
  auto signal = parser.signal();

  auto& log = logger(parser.outputPath());
  Coordinator coordinator(id, barrier, signal);
  udp::Socket socket(local_port(parser));
  msg::Broadcast broadcast(parser, socket, log);

  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();

  std::cout << "Broadcasting messages...\n\n";

  for (int i = 0; i < 10; ++i) {
    broadcast.send(i);
    log.log_broadcast(i);
  }

  constexpr auto timeout = std::chrono::seconds(3);
  while (true) {
    const auto start = std::chrono::steady_clock::now();
    do {
      msg::Message msg{};
      if (socket.try_recv(reinterpret_cast<char*>(&msg), sizeof(msg))
              .has_value()) {
        broadcast.receive(msg);
      }
    } while (std::chrono::steady_clock::now() - start < timeout);
    broadcast.resynchronize();
  }

  std::cout << "Signaling end of broadcasting messages\n\n";
  coordinator.finishedBroadcasting();

  return 0;
}
