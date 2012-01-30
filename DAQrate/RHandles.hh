#ifndef RHandles_hhh
#define RHandles_hhh

#include "DAQdata/Fragment.hh"
#include "DAQdata/Fragments.hh"
#include "Config.hh"

#include <vector>

#include "quiet_mpi.hh"

/*
  Protocol: want to do a send for each request object, then wait for for
  pending requests to complete, followed by a reset to allow another set
  of sends to be completed.

  This needs to be separated into a thing for sending and a thing for receiving.
  There probably needs to be a common class that both use.
 */
 
class RHandles
{
public:
  typedef std::vector<MPI_Request> Requests;
  typedef std::vector<MPI_Status> Statuses;
  typedef std::vector<int> Flags; // busy flags

  explicit RHandles(Config const&);

  // will take the data on the send (not copy),
  // will replace the data on recv (not copy)
  void recvEvent(artdaq::Fragment&);
  void waitAll();

private:

  int buffer_count_; // was size_
  int fragment_words_;
  int src_count_; // number of sources
  int src_start_; // start of the source ranks
  int rank_;

  Requests reqs_;
  Statuses stats_;
  Flags flags_;

  artdaq::Fragments frags_;
  bool is_direct_;
  int friend_;
};

#endif

