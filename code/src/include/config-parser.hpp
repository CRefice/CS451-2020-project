#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <locale>

#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "message.hpp"
#include "static-vec.hpp"

class ConfigParser {
public:
  ConfigParser(const std::string& config_file);

  struct Dependency {
    msg::ProcessId process;
    StaticVec<msg::ProcessId, msg::MAX_PROCESSES> depends;
  };

  msg::BroadcastSeqNum num_messages() const { return m; }
  std::vector<Dependency> dependencies() const { return deps; }

private:
  void parse(std::ifstream& file);
  void parse_message_num(const std::string& line);
  void parse_dependency(const std::string& line);

  msg::BroadcastSeqNum m = 0;
  std::vector<Dependency> deps;
};
