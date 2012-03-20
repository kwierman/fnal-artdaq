
#include "artdaq/DAQrate/RHandles.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQrate/Debug.hh"
#include "artdaq/DAQrate/MPITag.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "cetlib/container_algorithms.h"

#include <cassert>
#include <fstream>
#include <sstream>

// size_ = number of buffers
// fragment_size_ = number of longs in a fragment of an event
//
// be careful about the event/fragment size - hardwired to sizeof(long)
// and should be determined from the FragmentPool

artdaq::RHandles::RHandles(size_t buffer_count,
                           uint64_t max_initial_send_words,
                           size_t src_count,
                           size_t src_start):
  buffer_count_(buffer_count),
  max_initial_send_words_(max_initial_send_words),
  src_count_(src_count),
  src_start_(src_start),
  reqs_(buffer_count_, MPI_REQUEST_NULL),
  stats_(buffer_count_),
  flags_(buffer_count_),
  payload_(buffer_count_)
{
  // post all the buffers, making sure that the source numbers are correct
  for (int i = 0; i < buffer_count_; ++i) {
    // make sure all buffers are the correct size
    payload_[i].resize(max_initial_send_words_);
    // Sources are [0,offset). This works if src_count == 1.
    int from = i % src_count_ + src_start_;
    Debug << "sink posting buffer " << i << " size=" << max_initial_send_words_
          << " from=" << from << flusher;
    MPI_Irecv(&(payload_[i])[0],
              (max_initial_send_words_ * sizeof(Fragment::value_type)),
              MPI_BYTE,
              from,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &reqs_[i]);
  }
}

static void printError(int rc, int which, MPI_Status &)
{
  if (rc != MPI_SUCCESS) {
    char err_buffer[200];
    int errclass, reslen = sizeof(err_buffer);
    MPI_Error_class(rc, &errclass);
    MPI_Error_string(rc, err_buffer, &reslen);
    Debug << "rec printError:"
          << " rc=" << rc
          << " which=" << which
          << " err=" << err_buffer
          << flusher;
  }
#if 0
  Debug << " MPI_ERROR=" << stat.MPI_ERROR
        << " MPI_SOURCE=" << stat.MPI_SOURCE
        << flusher;
#endif
}

void artdaq::RHandles::recvEvent(Fragment & output)
{
  // Debug << "recv entered" << flusher;
  RecvMeas rm;
  int which;
  MPI_Waitany(buffer_count_, &reqs_[0], &which, &stats_[0]);
  if (which == MPI_UNDEFINED)
  { throw "NOTE: MPI_UNDEFINED returned as on index value from Waitany"; }
  if (reqs_[which] != MPI_REQUEST_NULL)
  { throw "NOTE: req is not MPI_REQUEST_NULL in recvEvent"; }
  if (stats_[which].MPI_TAG == MPITag::FINAL) { // Done.
    // The Fragment at index 'which' is now available.
    // Resize (down) to size to remove trailing garbage.
    payload_[which].resize(payload_[which].size() -
                           detail::RawFragmentHeader::num_words());;
    output.swap(payload_[which]);
    // Reset our buffer.
    Fragment tmp(max_initial_send_words_);
    payload_[which].swap(tmp);
    // Performance measurement.
    Fragment::event_id_t event_id = output.eventID();
    rm.woke(event_id, which);
    int rank;
    Debug << "recv: "
          << (MPI_Comm_rank(MPI_COMM_WORLD, &rank), rank)
          << " id=" << event_id
          << " fragID=" << output.fragmentID()
          << " which=" << which
          << flusher;
    if (output.type() != Fragment::type_t::END_OF_DATA) {
      // Repost the request that was complete, from the same sender.  This
      // makes the buffer we've just received data on available to receive
      // new data.
      MPI_Irecv(&(payload_[which])[0],
                (max_initial_send_words_ * sizeof(Fragment::value_type)),
                MPI_BYTE,
                stats_[which].MPI_SOURCE, // Same source.
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &reqs_[which]);
      rm.post(stats_[which].MPI_SOURCE);
    } // else done.
  }
  else { // Incomplete.
    assert(stats_[which].MPI_TAG == MPITag::INCOMPLETE);
    // Make our frag the correct size to receive the reset of the data.
    payload_[which].resize(payload_[which].size() -
                         detail::RawFragmentHeader::num_words());;
    // TODO: Should I call rm.woke() here or not?
    // Repost to receive the rest of the data (all in one chunk since we
    // know how much to expect).
    MPI_Irecv(&(payload_[which])[max_initial_send_words_],
              (payload_[which].size() - max_initial_send_words_)
              * sizeof(Fragment::value_type),
              MPI_BYTE,
              stats_[which].MPI_SOURCE,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &reqs_[which]);
    recvEvent(output); // Call ourselves to wait for the final chunk.
  }
}

void artdaq::RHandles::waitAll()
{
  // clean up the remaining buffers
  for (int i = 0; i < buffer_count_; ++i) {
    MPI_Cancel(&reqs_[i]);
    MPI_Wait(&reqs_[i], &stats_[i]);
  }
}
