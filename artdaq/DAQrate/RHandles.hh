#ifndef artdaq_DAQrate_RHandles_hh
#define artdaq_DAQrate_RHandles_hh

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/Fragments.hh"

#include <vector>

#include "artdaq/DAQrate/quiet_mpi.hh"

/*
  Protocol: want to do a send for each request object, then wait for for
  pending requests to complete, followed by a reset to allow another set
  of sends to be completed.

  This needs to be separated into a thing for sending and a thing for receiving.
  There probably needs to be a common class that both use.
 */

namespace artdaq {
  class RHandles;
}

class artdaq::RHandles {
public:
  typedef std::vector<MPI_Request> Requests;
  typedef std::vector<int> Flags; // busy flags

  RHandles(size_t buffer_count,
           uint64_t max_initial_send_words,
           size_t src_count,
           size_t src_start);

  // will take the data on the send (not copy),
  // will replace the data on recv (not copy)
  size_t recvFragment(Fragment &); // Return rank of source of fragment.
  void waitAll();

private:
  int nextSource_();

  int buffer_count_; // was size_
  int max_initial_send_words_;
  int src_count_; // number of sources
  int src_start_; // start of the source ranks

  Requests reqs_;
  Flags flags_;
  int last_source_posted_;

  Fragments payload_;
};

inline
int
artdaq::RHandles::nextSource_()
{
  return ++last_source_posted_ % src_count_ + src_start_;
}

#endif /* artdaq_DAQrate_RHandles_hh */

