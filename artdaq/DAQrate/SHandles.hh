#ifndef artdaq_DAQrate_SHandles_hh
#define artdaq_DAQrate_SHandles_hh

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/MPITag.hh"

#include <vector>

#include "artdaq/DAQrate/quiet_mpi.hh"

// SHandles handles the transmission of Fragments to one or more receivers in a
// round-robin fashion. Multiple receivers must have consecutive rank numbers.

namespace artdaq {
  class SHandles;
}

class artdaq::SHandles {
public:
  typedef std::vector<MPI_Request> Requests;
  typedef std::vector<MPI_Status> Statuses;
  typedef std::vector<int> Flags; // busy flags

  // buffer_count is the number of MPI_Request objects that will be used.
  // Fragments with dataSize() greater than max_payload_size will not be send.
  // dest_count is the number of receivers used in the round-robin algorithm
  // dest_start is the rank of the first receiver
  SHandles(size_t buffer_count,
           uint64_t max_payload_size,
           size_t dest_count,
           size_t dest_start);

  // Send the given Fragment. Return the rank of the destination to which
  // the Fragment was sent.
  size_t sendFragment(Fragment &&);

  // Send an EOF Fragment to the receiver at rank dest;
  // the EOF Fragment will report that numFragmentsSent
  // fragments have been sent before this one.
  void sendEODFrag(size_t dest, size_t numFragmentsSent);

  // Wait for all the data transfers scheduled by calls
  // to MPI_Isend to finish, then return.
  void waitAll();

private:

  size_t calcDest(Fragment::sequence_id_t) const;
  size_t findAvailable();
  void sendFragTo(Fragment && frag,
                  size_t dest);

  size_t const buffer_count_;
  uint64_t const max_payload_size_;
  size_t const dest_count_;
  size_t const dest_start_;
  size_t pos_; // next slot to check

  Requests reqs_;
  Statuses stats_;
  Flags flags_;

  Fragments payload_;
};

#endif /* artdaq_DAQrate_SHandles_hh */
