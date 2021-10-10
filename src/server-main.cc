#include "node.hh"
#include "leader-election.hh"

int
main(int argc, char* argv[]) {
  Node node{};

  node.init();

  node.startMainLoops();

  node.destroy();

  return 0;
}
