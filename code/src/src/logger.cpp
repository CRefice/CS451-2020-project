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
  file.close();
}

void Logger::log_broadcast(int seq_num) {
  buffer += "b ";
  buffer += std::to_string(seq_num);
  buffer += '\n';
}

void Logger::deliver(const msg::Message& msg) {
  buffer += "d ";
  buffer += std::to_string(msg.broadcast_id.sender);
  buffer += ' ';
  buffer += std::to_string(msg.broadcast_id.sequence_num);
  buffer += '\n';
}
