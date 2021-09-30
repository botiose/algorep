#include <mpi.h>

#include "node.hh"

int
main(int argc, char* argv[]) {
  Node node{};

  node.startMessenger();

  node.startReceiveLoop();

  node.stopMessenger();

  return 0;
}
