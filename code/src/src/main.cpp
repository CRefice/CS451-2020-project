#include <chrono>
#include <iostream>
#include <thread>

#include <csignal>

#include "barrier.hpp"
#include "links.hpp"
#include "parser.hpp"

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";

  // write/flush output file if necessary
  std::cout << "Writing output.\n";

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

  Coordinator coordinator(id, barrier, signal);
  MessageLink link(parser);

  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();

  std::cout << "Broadcasting messages...\n\n";

  auto neighbor_id = 1 + (id % hosts.size());

  for (int i = 0; i < 10; ++i) {
    link.send_to(neighbor_id, i);
    std::cout << "b " << i << '\n';
    if (auto msg = link.try_recv()) {
      std::cout << "d " << msg->sender << ' ' << msg->sequence_num << '\n';
    }
  }

  constexpr auto timeout = std::chrono::milliseconds(200);
  while (true) {
    const auto start = std::chrono::steady_clock::now();
    do {
      if (auto msg = link.try_recv()) {
        std::cout << "d " << msg->sender << ' ' << msg->sequence_num << '\n';
      }
    } while (std::chrono::steady_clock::now() - start < timeout);
    link.send_heartbeats();
  }

  std::cout << "Signaling end of broadcasting messages\n\n";
  coordinator.finishedBroadcasting();

  return 0;
}
