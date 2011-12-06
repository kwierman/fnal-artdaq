#ifndef Mpi_HHH
#define Mpi_HHH

#include "mpi.h"

class MPIProg
{
public:
  MPIProg(int& argc, char**& argv);
  ~MPIProg();

  int procs_;
  int rank_;
};


#endif
