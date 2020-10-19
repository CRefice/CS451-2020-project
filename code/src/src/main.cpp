#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include "barrier.hpp"
#include "broadcast.hpp"
#include "logger.hpp"
#include "parser.hpp"

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
  msg::FairLossLink link(parser);
  msg::UniformReliableBroadcast broadcast(parser, link, log);

  unsigned int n = 10;
  if (parser.configPath()) {
    std::ifstream file(parser.configPath());
    if (!file.is_open()) {
      throw std::runtime_error("couldn't open config file " +
                               std::string(parser.configPath()));
    }
    file >> n;
  }

  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();

  for (auto i = 1u; i <= n; ++i) {
    broadcast.send(i);
    log.log_broadcast(i);
    link.try_deliver();
  }

  const std::size_t total_messages = n * hosts.size();
  while (log.received_count() < total_messages) {
    link.deliver();
  }

  std::cout << "Signaling end of broadcasting messages\n\n";
  coordinator.finishedBroadcasting();

  std::cout << "Writing output.\n\n";
  logger(nullptr).flush();
}
