#include "artdaq/DAQrate/RHandles.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQdata/Debug.hh"
#include "artdaq/DAQrate/MPITag.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"

#include <cassert>
#include <fstream>
#include <sstream>

artdaq::RHandles::RHandles(size_t buffer_count,
                           uint64_t max_initial_send_words,
                           size_t src_count,
                           size_t src_start):
  buffer_count_(buffer_count),
  max_initial_send_words_(max_initial_send_words),
  src_count_(src_count),
  src_start_(src_start),
  reqs_(buffer_count_, MPI_REQUEST_NULL),
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

size_t
artdaq::RHandles::
recvFragment(Fragment & output)
{
  // Debug << "recv entered" << flusher;
  RecvMeas rm;
  int which;
  MPI_Status status = { 0, 0, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_SUCCESS };
  int wait_result = MPI_Waitany(buffer_count_, &reqs_[0], &which, &status);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (which == MPI_UNDEFINED)
  { throw "NOTE: MPI_UNDEFINED returned as on index value from Waitany"; }
  if (reqs_[which] != MPI_REQUEST_NULL)
  { throw "NOTE: req is not MPI_REQUEST_NULL in recvFragment"; }
  Fragment::sequence_id_t sequence_id = payload_[which].sequenceID();
  Debug << "recv: " << rank
        << " idx=" << which
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
  payload_[which].resize(payload_[which].size() -
                         detail::RawFragmentHeader::num_words());;
  output.swap(payload_[which]);
  // Reset our buffer.
  Fragment tmp(max_initial_send_words_);
  payload_[which].swap(tmp);
  // Performance measurement.
  rm.woke(sequence_id, which);
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
  rm.post(status.MPI_SOURCE);
  return status.MPI_SOURCE;
}

void artdaq::RHandles::waitAll()
{
  // clean up the remaining buffers
  for (int i = 0; i < buffer_count_; ++i) {
    int result = MPI_Cancel(&reqs_[i]);
    if (result == MPI_SUCCESS) {
      MPI_Status status;
      MPI_Wait(&reqs_[i], &status);
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
