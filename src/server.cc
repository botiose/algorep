#include <string>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include "mpi.h"

int
main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  MPI_Finalize();

  return 0;
}
