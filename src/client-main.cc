#include "client.hh"

int
main(int argc, char* argv[]) {
  Client client{};

  client.connect();

  client.disconnect();

  return 0;
}

