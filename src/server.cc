#include <mpi.h>

#include "node.hh"
#include "leader-election.hh"

int
main(int argc, char* argv[]) {
  Node node{};

  node.init();

  // TODO remove
  if (node.isLeader() == true) {
    node.replicateData("Hello World");
  }

  // TODO rename
  node.startMainLoops();

  node.destroy();

  return 0;
}
