#ifndef artdaq_DAQrate_MPIProg_hh
#define artdaq_DAQrate_MPIProg_hh

#include "artdaq/DAQrate/quiet_mpi.hh"

struct MPIProg {
  MPIProg(int argc, char ** argv)
    {
      MPI_Init(&argc, &argv);
      MPI_Comm_size(MPI_COMM_WORLD, &procs_);
      MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
    };
  ~MPIProg() { MPI_Finalize(); };

  int procs_;
  int rank_;
};


#endif /* artdaq_DAQrate_MPIProg_hh */
