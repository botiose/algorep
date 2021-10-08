#include <mpi.h>

#include "node.hh"
#include "leader-election.hh"

int
main(int argc, char* argv[]) {
  Node node{};

  // TODO remove
  node.startMessenger();

  // TODO remove
  if (node.isLeader() == true) {
    node.startAcceptThread();
   
    // TODO remove; Used during development of the paxos algorithm. 
    node.replicateData("Hello World");
  }

  // TODO rename
  node.startReceiveLoops();

  // TODO remove
  node.stopMessenger();

  return 0;
}
