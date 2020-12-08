#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include "barrier.hpp"
#include "concurrent-queue.hpp"
#include "config-parser.hpp"
#include "fifo-broadcast.hpp"
#include "localized-causal-broadcast.hpp"
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

  ConfigParser config(parser.configPath());
  auto num_messages = config.num_messages();

  std::cout << "#Messages: " << num_messages << '\n';

  auto& log = logger(parser.outputPath());
  Coordinator coordinator(id, barrier, signal);
  udp::Socket socket(local_port(parser));
  msg::FairLossLink link(parser, socket);
  msg::LocalizedCausalBroadcast broadcast(config, parser, link, log);

  ConcurrentQueue<msg::Message> message_queue;
  auto receiver = Task([&socket, &message_queue](Task::CancelToken& cancel) {
    while (!cancel) {
      msg::Message msg{};
      auto len = socket.recv(reinterpret_cast<char*>(&msg), sizeof(msg));
      const auto vector_size = (len - offsetof(msg::Message, vector_clock)) /
                               sizeof(msg::BroadcastSeqNum);
      msg.vector_clock.force_set_size(vector_size);
      message_queue.push(msg);
    }
  });

  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();

  msg::BroadcastSeqNum next_msg = 0;
  std::size_t subsequent_recvs = num_messages;
  while (next_msg < num_messages) {
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

  auto to_receive = num_messages * hosts.size();
  while (log.received_count() < to_receive) {
    link.receive(message_queue.pop());
  }

  std::cout << id << " finished receiving.\n";

  while (true) {
  }
}
