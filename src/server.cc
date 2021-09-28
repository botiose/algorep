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

  if (node.getNodeId() == 4) {
    node.startElection();
  } else {
    int srcNodeId;
    Message message;
    node.receiveMessage(srcNodeId, message);

    std::cout << "node: " << node.getNodeId()
              << " received message with tag: " << message.tag
              << " and code: " << message.code << " from node: " << srcNodeId
              << std::endl;
  }

  MPI_Finalize();

  return 0;
}
