#include <string>
#include <iostream>
#include <stdio.h>
#include "mpi.h"

int
main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char port[MPI_MAX_PORT_NAME];
  MPI_Open_port(MPI_INFO_NULL, &port[0]);
  
  MPI_Info scopeInfo;
  MPI_Info_create(&scopeInfo);
  MPI_Info_set(scopeInfo, "ompi_global_scope", "true");

  MPI_Publish_name("server", scopeInfo, port);
  std::cout << "server " << rank << " waiting for connection." << std::endl; 

  MPI_Comm interComm;
  MPI_Comm_accept(port, MPI_INFO_NULL, rank, MPI_COMM_WORLD, &interComm);

  std::cout << "server " << rank << " connected." << std::endl; 
  std::cout << "server " << rank << " shutting down." << std::endl; 

  MPI_Unpublish_name("server", scopeInfo, port);

  MPI_Close_port(port);

  MPI_Finalize();
  return 0;
}
