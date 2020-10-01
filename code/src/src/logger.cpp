#include <fstream>

#include "logger.hpp"

void Logger::flush() { file << buffer; }

void Logger::log_broadcast(int seq_num) {
  buffer += "b ";
  buffer += std::to_string(seq_num);
  buffer += '\n';
}

void Logger::deliver(const msg::Message& msg) {
  buffer += "d ";
  buffer += std::to_string(msg.id.sender);
  buffer += ' ';
  buffer += std::to_string(msg.broadcast_seq_num);
  buffer += '\n';
}
