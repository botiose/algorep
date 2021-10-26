#include "client.hh"

int
main(int argc, char* argv[]) {
  Client client{};

  client.init(argc, argv);

  client.replicateCommands();

  client.destroy();

  return 0;
}

