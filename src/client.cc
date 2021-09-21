#include <iostream>
#include <stdio.h>
#include "mpi.h"

int
main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char port[MPI_MAX_PORT_NAME];

  MPI_Lookup_name("server", MPI_INFO_NULL, &port[0]);

  MPI_Comm interComm;
  MPI_Comm_connect(port, MPI_INFO_NULL, rank, MPI_COMM_WORLD, &interComm);

  std::cout << "client " << rank << " connected." << std::endl; 
  std::cout << "client " << rank << " shutting down." << std::endl; 

  MPI_Comm_disconnect(&interComm);

  MPI_Finalize();
  return 0;
}

