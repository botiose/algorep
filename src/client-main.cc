#include <chrono>
#include <thread>

#include "client.hh"

int
main(int argc, char* argv[]) {
  Client client{};

  client.connect(argc, argv);

  client.replicate("Hello World");

  client.disconnect();

  return 0;
}

