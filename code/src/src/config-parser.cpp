#include "config-parser.hpp"

#include <sstream>

ConfigParser::ConfigParser(const std::string& config_file) {
  std::ifstream file(config_file);
  if (!file.is_open()) {
    throw std::runtime_error("couldn't open config file " + config_file);
  }
  parse(file);
}

void ConfigParser::parse(std::ifstream& file) {
  std::string line;
  std::getline(file, line);
  parse_message_num(line);
  while (std::getline(file, line)) {
    parse_dependency(line);
  }
}

void ConfigParser::parse_message_num(const std::string& line) {
  std::istringstream iss(line);
  iss >> m;
}

void ConfigParser::parse_dependency(const std::string& line) {
  std::istringstream iss(line);
  Dependency dep;
  iss >> dep.process;
  msg::ProcessId process{};
  while (iss >> process) {
    dep.depends.push_back(process);
  }
}
