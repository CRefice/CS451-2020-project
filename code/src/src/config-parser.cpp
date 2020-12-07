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
  std::sort(deps.begin(), deps.end(),
            [](const Dependency& a, const Dependency& b) {
              return a.process < b.process;
            });
}

void ConfigParser::parse_message_num(const std::string& line) {
  std::istringstream iss(line);
  iss >> m;
}

void ConfigParser::parse_dependency(const std::string& line) {
  std::istringstream iss(line);
  int process{};
  iss >> process;
  Dependency dep;
  dep.process = static_cast<msg::ProcessId>(process);
  while (iss >> process) {
    dep.depends.push_back(static_cast<msg::ProcessId>(process));
  }
  deps.push_back(dep);
}
