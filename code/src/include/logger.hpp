#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "message.hpp"

class Logger : public msg::Observer {
public:
  Logger(const char* path) : file(path, std::ios::trunc) {}

  void log_broadcast(int seq_num);
  void flush();

private:
  void deliver(const msg::Message& msg) override;

  std::ofstream file;
  std::string buffer;
};
