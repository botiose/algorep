#include <string>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include "mpi.h"

int
main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char port[MPI_MAX_PORT_NAME];
  MPI_Open_port(MPI_INFO_NULL, &port[0]);
  
  MPI_Info publishInfo;
  MPI_Info_create(&publishInfo);
  MPI_Info_set(publishInfo, "ompi_global_scope", "true");
  MPI_Info_set(publishInfo, "ompi_unique", "true");

  std::cout << "server " << rank << " openend port: " << port << std::endl; 

  std::cout << "server " << rank << " publishing port: " << port << std::endl; 
  MPI_Publish_name("server", publishInfo, port);

  std::cout << "server " << rank << " waiting for connection." << std::endl; 
  MPI_Comm interComm;
  MPI_Comm_accept(port, MPI_INFO_NULL, 0, MPI_COMM_SELF, &interComm);

  std::cout << "server " << rank << " connected." << std::endl; 

  MPI_Info unpublishInfo;
  MPI_Info_create(&unpublishInfo);
  MPI_Info_set(unpublishInfo, "ompi_global_scope", "true");

  std::cout << "server " << rank << " shutting down." << std::endl; 
  MPI_Unpublish_name("server", unpublishInfo, port);

  MPI_Close_port(port);

  MPI_Finalize();
  return 0;
}
