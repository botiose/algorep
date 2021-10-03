#include <mpi.h>

#include "node.hh"
#include "leader-election.hh"

int
main(int argc, char* argv[]) {
  Node node{};

  node.startMessenger();

  if (node.isLeader() == true) {
    node.startAcceptThread();
  }

  // TODO remove; Used during development of the paxos algorithm.
  node.replicate("Hello World");

  node.startReceiveLoop();

  node.stopMessenger();

  return 0;
}
