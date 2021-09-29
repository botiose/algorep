#include <mpi.h>

#include "node.hh"

int
main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  Node node(rank, size);

  if (node.getNodeId() == 1) {
    node.startElection();
  }

  node.startReceiveLoop();

  MPI_Finalize();

  return 0;
}
