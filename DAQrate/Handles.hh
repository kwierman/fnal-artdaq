#ifndef Handles_hhh
#define Handles_hhh

#include "EventPool.hh"
#include "Config.hh"

#include <vector>

#include "mpi.h"

/*
  Protocol: want to do a send for each request object, then wait for for
  pending requests to complete, followed by a reset to allow another set
  of sends to be completed.

  This needs to be separated into a thing for sending and a thing for receiving.
  There probably needs to be a common class that both use.
 */
 
class Handles
{
public:
  typedef std::vector<MPI_Request> Requests;
  typedef std::vector<MPI_Status> Statuses;
  typedef std::vector<int> Flags; // busy flags

  typedef FragmentPool::Data Data;
  typedef std::vector<Data> Events;

  Handles(Config const&);

  void waitAll();
  void cleanup();
  int dest(long event_id) const;

private:

  int size_;
  int fragment_size_;
  int sinks_;
  Requests reqs_;
  Statuses stats_;
  Flags flags_;
  Events events_;
  int offset_;
  int rank_;
  int pos_;
};

#endif

