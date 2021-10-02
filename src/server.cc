#include <mpi.h>

#include "node.hh"
#include "leader-election.hh"

int
main(int argc, char* argv[]) {
  Node node{};

  node.startMessenger();

  node.startReceiveLoop();

  node.stopMessenger();

  return 0;
}
