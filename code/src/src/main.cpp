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

  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();

  MessageLink link(parser);
  std::cout << "Broadcasting messages...\n\n";

  while (true) {
    auto neighbor_id = 1 + (id % parser.hosts().size());
    link.send_to(neighbor_id);
    std::cout << "d " << neighbor_id << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "b " << neighbor_id << '\n';
    auto msg = link.recv();
  }

  std::cout << "Signaling end of broadcasting messages\n\n";
  coordinator.finishedBroadcasting();

  return 0;
}
