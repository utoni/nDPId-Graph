#include <imweb.hpp>
#include <iostream>
#include <thread>

#include <math.h>

#include "controller.hpp"

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: client <host> <port>" << std::endl;
    return 1;
  }

  Controller c;

  if (!c.start(argv[1], argv[2])) {
    std::cerr << "Controller startup failed" << std::endl;
    return 1;
  }

  c.loop();

  return 0;
}
