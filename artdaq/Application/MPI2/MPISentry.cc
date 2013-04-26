#include "artdaq/Application/MPI2/MPISentry.hh"
#include "artdaq/DAQrate/quiet_mpi.hh"

artdaq::MPISentry::
MPISentry(int * argc_ptr, char *** argv_ptr)
:
  threading_level_(0),
  rank_(-1),
  procs_(0)
{
  MPI_Init(argc_ptr, argv_ptr);
  initialize_();
}

artdaq::MPISentry::
MPISentry(int * argc_ptr,
          char *** argv_ptr,
          int threading_level)
  :
  threading_level_(0),
  rank_(-1),
  procs_(0)
{
  MPI_Init_thread(argc_ptr, argv_ptr, threading_level, &threading_level_);
  initialize_();
}

artdaq::MPISentry::
~MPISentry()
{
  MPI_Finalize();
}

int
artdaq::MPISentry::
threading_level() const
{
  return threading_level_;
}

int
artdaq::MPISentry::
rank() const
{
  return rank_;
}

int
artdaq::MPISentry::
procs() const
{
  return procs_;
}

void
artdaq::MPISentry::
initialize_() {
  MPI_Comm_size(MPI_COMM_WORLD, &procs_);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
}
