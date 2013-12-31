#ifndef artdaq_Application_MPI2_MPISentry_hh
#define artdaq_Application_MPI2_MPISentry_hh

// #include <iostream>
// #include <boost/program_options.hpp>
// #include <boost/lexical_cast.hpp>
// #include "messagefacility/MessageLogger/MessageLogger.h"
// #include "artdaq/Application/configureMessageFacility.hh"
// #include "artdaq/Application/TaskType.hh"
// #include "artdaq/Application/MPI2/EventBuilderApp.hh"
// #include "artdaq/ExternalComms/xmlrpc_commander.hh"
// #include "artdaq/Application/MPI2/MPISentry.hh"
// #include "artdaq/DAQrate/quiet_mpi.hh"


#include "artdaq/Application/TaskType.hh"
#include "artdaq/DAQrate/quiet_mpi.hh"

namespace artdaq {
  class MPISentry;
}

class artdaq::MPISentry {

public:
  MPISentry(int * argc_ptr, char *** argv_ptr);
  MPISentry(int * argc_ptr,
            char *** argv_ptr,
            int threading_level);
  ~MPISentry();

  int threading_level() const;
  int rank() const;
  int procs() const;

  void create_local_group(artdaq::TaskType task);
  const MPI_Comm& local_group() const { return local_group_comm_; }

private:
  void initialize_();

  int threading_level_;
  int rank_;
  int procs_;

  MPI_Comm local_group_comm_;
};


#endif /* artdaq_Application_MPI2_MPISentry_hh */
