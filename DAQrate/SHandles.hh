#ifndef SHandles_hhh
#define SHandles_hhh

#include "FragmentPool.hh"
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
 
class SHandles
{
public:
  typedef std::vector<MPI_Request> Requests;
  typedef std::vector<MPI_Status> Statuses;
  typedef std::vector<int> Flags; // busy flags

  typedef FragmentPool::Data Data;
  typedef std::vector<Data> Fragments; // was Events

  SHandles(Config const&);

  // will take the data on the send (not copy),
  // will replace the data on recv (not copy)
  void sendEvent(Data&);
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

  Fragments frags_;
  bool is_direct_;
  int friend_;
};

#endif

