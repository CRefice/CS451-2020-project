#include <fstream>
#include <stdexcept>

#include "logger.hpp"

Logger::Logger(const char* path) : file(path) {
  if (!file) {
    throw std::runtime_error(std::string("couldn't open file ") + path +
                             " for writing");
  }
}

void Logger::flush() {
  file << buffer;
  file.flush();
  buffer.clear();
}

void Logger::log_broadcast(unsigned int seq_num) {
  log("b " + std::to_string(seq_num));
}

void Logger::deliver(const msg::Message& msg) {
  std::string line = "d ";
  line += std::to_string(msg.broadcast_id.sender);
  line += ' ';
  line += std::to_string(msg.broadcast_id.sequence_num);
  log(std::move(line));
  count++;
}

void Logger::log(std::string&& line) {
  static constexpr std::size_t MAX_LEN = 1024 * 1024; // 1MB
  line += '\n';
  if (buffer.size() + line.size() > MAX_LEN) {
    flush();
  }
  buffer += line;
}
