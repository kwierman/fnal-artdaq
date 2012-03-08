#ifndef artdaq_DAQrate_SHandles_hh
#define artdaq_DAQrate_SHandles_hh

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/Config.hh"

#include <vector>

#include "artdaq/DAQrate/quiet_mpi.hh"

/*
  Protocol: want to do a send for each request object, then wait for for
  pending requests to complete, followed by a reset to allow another set
  of sends to be completed.

  This needs to be separated into a thing for sending and a thing for receiving.
  There probably needs to be a common class that both use.
 */
 
class SHandles
{
public:
  typedef std::vector<MPI_Request> Requests;
  typedef std::vector<MPI_Status> Statuses;
  typedef std::vector<int> Flags; // busy flags

  explicit SHandles(Config const&);

  // will take the data on the send (not copy),
  // will replace the data on recv (not copy)
  void sendEvent(artdaq::Fragment&);
  void waitAll();
  
private:

  int dest(long event_id) const;
  int findAvailable();

  int buffer_count_; // size_
  int fragment_words_;
  int dest_count_;
  int dest_start_;
  int rank_; // my rank
  int pos_; // next slot to check

  Requests reqs_;
  Statuses stats_;
  Flags flags_;

  artdaq::Fragments frags_;
  bool is_direct_;
  int friend_;
};

#endif /* artdaq_DAQrate_SHandles_hh */
