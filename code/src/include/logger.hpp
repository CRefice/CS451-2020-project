#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "message.hpp"

class Logger : public msg::Observer {
public:
  Logger(const char* path);

  void log_broadcast(msg::BroadcastSeqNum seq_num);
  void flush();

  std::size_t received_count() const noexcept { return count; }

private:
  void deliver(const msg::Message& msg) override;

  void try_flush();

  std::ofstream file;
  std::ostringstream buffer;
  std::size_t count = 0;
};
