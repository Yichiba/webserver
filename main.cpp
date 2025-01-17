#include "main.h"
#include "server/Server.hpp"

int main(int argc, char __unused **argv) {
  try {
    if (argc >= 2)
      SyntaxError(argv[1]);
    Server server;
    if (argc >= 2)
      server.parsConfigFile(argv[1]);
    else
      server.generateDefaultConfig();
    server.runServer();
  } catch (std::exception & e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
