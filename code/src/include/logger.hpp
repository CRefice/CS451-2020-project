#pragma once

#include <iostream>

#include "message.hpp"

class Logger : public msg::Observer {
  void deliver(const msg::Message& msg) override {
    std::cout << "d " << msg.id.sender << ' ' << msg.broadcast_seq_num << '\n';
  }
};
