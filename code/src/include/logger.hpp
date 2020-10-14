#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "message.hpp"

class Logger : public msg::Observer {
public:
  Logger(const char* path);

  void log_broadcast(unsigned int seq_num);
  void flush();

  std::size_t received_count() const noexcept { return count; }

private:
  void deliver(const msg::Message& msg) override;

  std::ofstream file;
  std::string buffer;
  std::size_t count = 0;
};
