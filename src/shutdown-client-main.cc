#include "client.hh"

int
main(int argc, char* argv[]) {
  Client client{};

  client.shutdownServer(argc, argv);

  client.disconnect();

  return 0;
}

