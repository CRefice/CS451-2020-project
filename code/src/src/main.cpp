#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include "barrier.hpp"
#include "concurrent-queue.hpp"
#include "fifo-broadcast.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include "task.hpp"

static std::uint16_t local_port(Parser& parser) {
  for (auto& host : parser.hosts()) {
    if (host.id == parser.id()) {
      return host.port;
    }
  }
  throw std::runtime_error("no host with the given id found in the hosts file");
}

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
  msg::FairLossLink link(parser, socket);
  msg::FifoBroadcast broadcast(parser, link, log);

  ConcurrentQueue<msg::Message> message_queue;
  auto receiver = Task([&socket, &message_queue](Task::CancelToken& cancel) {
    while (!cancel) {
      msg::Message msg{};
      socket.recv(reinterpret_cast<char*>(&msg), sizeof(msg));
      message_queue.push(msg);
    }
  });

  msg::BroadcastSeqNum num_messages = 10;
  if (parser.configPath()) {
    std::ifstream file(parser.configPath());
    if (!file.is_open()) {
      throw std::runtime_error("couldn't open config file " +
                               std::string(parser.configPath()));
    }
    file >> num_messages;
  }
  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();

  msg::BroadcastSeqNum next_msg = 1;
  std::size_t subsequent_recvs = num_messages;
  while (next_msg <= num_messages) {
    std::optional<msg::Message> maybe_msg;
    if (subsequent_recvs > 0 && (maybe_msg = message_queue.try_pop())) {
      subsequent_recvs--;
      link.receive(*maybe_msg);
    } else {
      subsequent_recvs = num_messages;
      broadcast.send(next_msg);
      log.log_broadcast(next_msg);
      next_msg++;
    }
  }
  std::cout << "Signaling end of broadcasting messages\n\n";
  coordinator.finishedBroadcasting();

  while (true) {
    link.receive(message_queue.pop());
  }
}
