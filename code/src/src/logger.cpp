#include <fstream>
#include <iostream>
#include <stdexcept>

#include "logger.hpp"

Logger::Logger(const char* path) : file(path) {
  if (!file) {
    throw std::runtime_error(std::string("couldn't open file ") + path +
                             " for writing");
  }
}

void Logger::flush() {
  file << buffer.str();
  file.flush();
  buffer.clear();
  buffer.flush();
}

void Logger::log_broadcast(msg::BroadcastSeqNum seq_num) {
  buffer << "b " << seq_num << '\n';
}

void Logger::deliver(const msg::Message& msg) {
  buffer << "d " << +msg.originator << ' ' << msg.bcast_seq_num << '\n';
  count++;
  try_flush();
}

void Logger::try_flush() {
  static constexpr long MAX_LEN = 1024 * 1024; // 1MB
  if (buffer.tellp() > MAX_LEN) {
    flush();
  }
}
