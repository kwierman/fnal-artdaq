
#include "artdaq/DAQrate/RHandles.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQrate/Debug.hh"
#include "artdaq/DAQrate/MPITag.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"

#include <cassert>
#include <fstream>
#include <iterator>
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
  status_(),
  flags_(buffer_count_),
  last_source_posted_(-1),
  payload_(buffer_count_)
{
  // post all the buffers, making sure that the source numbers are correct
  for (int i = 0; i < buffer_count_; ++i) {
    // make sure all buffers are the correct size
    payload_[i].resize(max_initial_send_words_);
    int from = nextSource_();
    Debug << "Posting buffer " << i << " size=" << max_initial_send_words_
          << " for receive from=" << from << flusher;
    MPI_Irecv(&*payload_[i].headerBegin(),
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
  MPI_Waitany(buffer_count_, &reqs_[0], &which, &status_);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (which == MPI_UNDEFINED)
  { throw "NOTE: MPI_UNDEFINED returned as on index value from Waitany"; }
  if (reqs_[which] != MPI_REQUEST_NULL)
  { throw "NOTE: req is not MPI_REQUEST_NULL in recvEvent"; }
  Fragment::event_id_t event_id = payload_[which].eventID();
  Debug << "recv: " << rank
        << " idx=" << which
        << " status_error=" << status_.MPI_ERROR
        << " status_count=" << status_.count
        << " source=" << status_.MPI_SOURCE
        << " tag=" << status_.MPI_TAG
        << " Fragment eventID=" << event_id
        << " Fragment size=" << payload_[which].size()
        << " Fragment dataSize=" << payload_[which].dataSize()
        << " fragID=" << payload_[which].fragmentID()
        << flusher;
  if (status_.MPI_TAG == MPITag::FINAL) { // Done.
    // The Fragment at index 'which' is now available.
    // Resize (down) to size to remove trailing garbage.
    payload_[which].resize(payload_[which].size() -
                           detail::RawFragmentHeader::num_words());;
    std::copy_n(payload_[which].dataBegin(),
                std::min(static_cast<Fragment::value_type>(5), payload_[which].dataSize()),
                std::ostream_iterator<Fragment::value_type>(Debug, ", "));
    Debug << flusher;
    output.swap(payload_[which]);
    // Reset our buffer.
    Fragment tmp(max_initial_send_words_);
    payload_[which].swap(tmp);
    // Performance measurement.
    rm.woke(event_id, which);
    // Repost to receive more data from possibly different sources.
    int from = nextSource_();
    Debug << "Posting buffer " << which
          << " size=" << max_initial_send_words_
          << " for receive from=" << from
          << flusher;
    MPI_Irecv(&*payload_[which].headerBegin(),
              (max_initial_send_words_ * sizeof(Fragment::value_type)),
              MPI_BYTE,
              from,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &reqs_[which]);
    rm.post(status_.MPI_SOURCE);
  }
  else if (buffer_count_ > 1) {
    throw cet::exception("unimplemented")
      << "Currently unable to deal with overlarge fragments when "
      << "running with multiple buffers.";
  }
  else { // Incomplete.
    assert(status_.MPI_TAG == MPITag::INCOMPLETE);
    // Make our frag the correct size to receive the reset of the data.
    payload_[which].resize(payload_[which].size() -
                         detail::RawFragmentHeader::num_words());;
    // TODO: Should I call rm.woke() here or not?
    // Repost to receive the rest of the data (all in one chunk since we
    // know how much to expect).
    Debug << "Posting buffer " << which
          << " size=" << max_initial_send_words_
          << " for receive from=" << status_.MPI_SOURCE
          << flusher;
    MPI_Irecv(&*payload_[which].headerBegin() + max_initial_send_words_,
              (payload_[which].size() - max_initial_send_words_)
              * sizeof(Fragment::value_type),
              MPI_BYTE,
              status_.MPI_SOURCE,
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
    int result = MPI_Cancel(&reqs_[i]);
    if (result == MPI_SUCCESS) {
      MPI_Wait(&reqs_[i], &status_);
    }
    else {
      switch (result) {
      case MPI_ERR_REQUEST:
        throw "MPI_Cancel returned MPI_ERR_REQUEST";
      case MPI_ERR_ARG:
        throw "MPI_Cancel returned MPI_ERR_ARG";
      default:
        throw "MPI_Cancel returned unknown error code.";
      }
    }
  }
}
