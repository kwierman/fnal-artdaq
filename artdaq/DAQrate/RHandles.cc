#include "artdaq/DAQrate/RHandles.hh"

#include "art/Utilities/Exception.h"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQdata/Debug.hh"
#include "artdaq/DAQrate/MPITag.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"

artdaq::RHandles::RHandles(size_t buffer_count,
                           uint64_t max_payload_size,
                           size_t src_count,
                           size_t src_start):
  buffer_count_(buffer_count),
  max_payload_size_(max_payload_size),
  src_count_(src_count),
  src_start_(src_start),
  recv_frag_count_(src_count, src_start),
  src_status_(src_count, status_t::SENDING),
  expected_count_(src_count, 0),
  reqs_(buffer_count_, MPI_REQUEST_NULL),
  req_sources_(buffer_count_, MPI_ANY_SOURCE),
  last_source_posted_(-1),
  payload_(buffer_count_)
{
  Debug << "RHandles construction: "
        << buffer_count << " buffers, "
        << src_count << " sources starting at rank "
        << src_start << flusher;
  // Post all the buffers.
  for (size_t i = 0; i < buffer_count_; ++i) {
    // make sure all buffers are the correct size
    payload_[i].resize(max_payload_size_);
    post_(i, nextSource_());
  }
}

artdaq::RHandles::
~RHandles()
{
  waitAll_();
}

size_t
artdaq::RHandles::
recvFragment(Fragment & output)
{
  if (sourcesActive() == 0) {
    return MPI_ANY_SOURCE; // Nothing to do.
  }
  // Debug << "recv entered" << flusher;
  RecvMeas rm;
  int which;
  MPI_Status status;
  int wait_result = MPI_Waitany(buffer_count_, &reqs_[0], &which, &status);
  size_t src_index(indexForSource_(status.MPI_SOURCE));
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (which == MPI_UNDEFINED)
  { throw "NOTE: MPI_UNDEFINED returned as on index value from Waitany"; }
  if (reqs_[which] != MPI_REQUEST_NULL)
  { throw "NOTE: req is not MPI_REQUEST_NULL in recvFragment"; }
  Fragment::sequence_id_t sequence_id = payload_[which].sequenceID();
  Debug << "recv: " << rank
        << " idx=" << which
        << " Waitany_error=" << wait_result
        << " status_error=" << status.MPI_ERROR
        << " source=" << status.MPI_SOURCE
        << " tag=" << status.MPI_TAG
        << " Fragment_sequenceID=" << sequence_id
        << " Fragment_size=" << payload_[which].size()
        << " Fragment_dataSize=" << payload_[which].dataSize()
        << " fragID=" << payload_[which].fragmentID()
        << flusher;
  char err_buffer[MPI_MAX_ERROR_STRING];
  int resultlen;
  switch (wait_result) {
    case MPI_SUCCESS:
      break;
    case MPI_ERR_IN_STATUS:
      MPI_Error_string(status.MPI_ERROR, err_buffer, &resultlen);
      std::cerr << "Waitany ERROR: " << err_buffer << "\n";
      break;
    default:
      MPI_Error_string(wait_result, err_buffer, &resultlen);
      std::cerr << "Waitany ERROR: " << err_buffer << "\n";
  }
  // The Fragment at index 'which' is now available.
  // Resize (down) to size to remove trailing garbage.
  payload_[which].autoResize();
  output.swap(payload_[which]);
  // Reset our buffer.
  Fragment tmp(max_payload_size_);
  payload_[which].swap(tmp);
  // Performance measurement.
  rm.woke(sequence_id, which);
  // Fragment accounting.
  if (output.type() == Fragment::EndOfDataFragmentType) {
    src_status_[src_index] = status_t::PENDING;
    expected_count_[src_index] = *output.dataBegin();
    Debug << "Received EOD from source " << status.MPI_SOURCE
          << " (index " << src_index << ") expecting total of "
          << *output.dataBegin() << " fragments" << flusher;
  }
  else {
    recv_frag_count_.incSlot(status.MPI_SOURCE);
  }
  switch (src_status_[src_index]) {
    case status_t::PENDING:
      Debug << "Checking received count "
            << recv_frag_count_.slotCount(status.MPI_SOURCE)
            << " against expected total "
            << expected_count_[src_index]
            << flusher;
      if (recv_frag_count_.slotCount(status.MPI_SOURCE) ==
          expected_count_[src_index]) {
        src_status_[src_index] = status_t::DONE;
      }
      break;
    case status_t::DONE:
      throw art::Exception(art::errors::LogicError, "RHandles")
          << "Received extra fragments from source "
          << status.MPI_SOURCE
          << ".\n";
    case status_t::SENDING:
      break;
    default:
      throw art::Exception(art::errors::LogicError, "RHandles")
          << "INTERNAL ERROR: Unrecognized status_t value "
          << static_cast<int>(src_status_[src_index])
          << ".\n";
  }
  // Repost to receive more data.
  if (src_status_[src_index] == status_t::DONE) { // Just happened.
    if (sourcesActive() > 0) { // Post for input from a still-active source.
      int nextSource = nextSource_();
      rm.post(nextSource);
      post_(which, nextSource); // This buffer doesn't need cancelling.
    }
    else {
      req_sources_[which] = MPI_ANY_SOURCE; // Done with this buffer.
    }
    cancelAndRepost_(status.MPI_SOURCE); // Cancel and possibly repost.
  }
  else {
    rm.post(status.MPI_SOURCE);
    post_(which, status.MPI_SOURCE);
  }
  return status.MPI_SOURCE;
}

void
artdaq::RHandles::
waitAll_()
{
  // clean up the remaining buffers
  for (size_t i = 0; i < buffer_count_; ++i) {
    if (req_sources_[i] != MPI_ANY_SOURCE) {
      cancelReq_(i);
    }
  }
}

void
artdaq::RHandles::
cancelReq_(size_t buf)
{
  Debug << "Cancelling post for buffer "
        << buf
        << flusher;
  int result = MPI_Cancel(&reqs_[buf]);
  if (result == MPI_SUCCESS) {
    MPI_Status status;
    MPI_Wait(&reqs_[buf], &status);
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

void
artdaq::RHandles::
post_(size_t buf, size_t src)
{
  Debug << "Posting buffer " << buf
        << " size=" << payload_[buf].size()
        << " for receive src=" << src
        << flusher;
  MPI_Irecv(&*payload_[buf].headerBegin(),
            (payload_[buf].size() * sizeof(Fragment::value_type)),
            MPI_BYTE,
            src,
            MPI_ANY_TAG,
            MPI_COMM_WORLD,
            &reqs_[buf]);
  req_sources_[buf] = src;
  last_source_posted_ = src;
}

void
artdaq::RHandles::
cancelAndRepost_(size_t src)
{
  for (size_t i = 0; i < buffer_count_; ++i) {
    if (static_cast<int>(src) == req_sources_[i]) {
      cancelReq_(i);
      if (sourcesActive() > 0) { // Still busy.
        post_(i, nextSource_());
      }
      else {
        req_sources_[i] = MPI_ANY_SOURCE; // Done.
      }
    }
  }
}
