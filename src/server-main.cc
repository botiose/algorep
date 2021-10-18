
#include "node.hh"

int
main(int argc, char* argv[]) {
  Node node{};

  node.init(argc, argv);

  node.destroy();

  return 0;
}
