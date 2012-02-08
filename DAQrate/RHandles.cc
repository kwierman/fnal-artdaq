
#include "RHandles.hh"
#include "Perf.hh"
#include "Debug.hh"
#include "Utils.hh"
#include "DAQdata/RawData.hh"
#include "cetlib/container_algorithms.h"

#include <fstream>
#include <sstream>


using namespace std;
using namespace artdaq;

#define MY_TAG 2

// size_ = number of buffers
// fragment_size_ = number of longs in a fragment of an event
//
// be careful about the event/fragment size - hardwired to sizeof(long)
// and should be determined from the FragmentPool

RHandles::RHandles(Config const & conf):
  buffer_count_(conf.sink_buffer_count_),
  fragment_words_(conf.fragment_words_),
  src_count_(conf.srcCount()),
  src_start_(conf.srcStart()),
  rank_(conf.rank_),
  reqs_(buffer_count_, MPI_REQUEST_NULL),
  stats_(buffer_count_),
  flags_(buffer_count_),
  frags_(buffer_count_),
  is_direct_(conf.type_ == Config::TaskSource),
  friend_(conf.getSrcFriend())
{
  // post all the buffers, making sure that the source numbers are correct
  for (int i = 0; i < buffer_count_; ++i) {
    // make sure all buffers are the correct size
    frags_[i].resize(fragment_words_);
    // sources are [0,offset)
    int from = is_direct_ ? friend_ : (i % src_count_ + src_start_);
    Debug << "sink posting buffer " << i << " size=" << fragment_words_
          << " from=" << from << flusher;
    MPI_Irecv(&(frags_[i])[0], (fragment_words_ * sizeof(artdaq::RawDataType)),
              MPI_BYTE, from, MY_TAG, MPI_COMM_WORLD, &reqs_[i]);
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

void RHandles::recvEvent(Fragment & e)
{
  // Debug << "recv entered" << flusher;
  RecvMeas rm;
  int which;
#if 0
  int rc =
#endif
    MPI_Waitany(buffer_count_, &reqs_[0], &which, &stats_[0]);
#if 0
  printError(rc, which, stats_[which]);
  writeData(rank_, (const char*) & (frags_[which])[0], frags_[which].size()*sizeof(Data::value_type));
#endif
  if (which == MPI_UNDEFINED)
  { throw "NOTE: MPI_UNDEFINED returned as on index value from Waitany"; }
  if (reqs_[which] != MPI_REQUEST_NULL)
  { throw "NOTE: req is not MPI_REQUEST_NULL in recvEvent"; }
  // event at which is now available
  e.clear();
  e.reserve(frags_[which].size());
  cet::for_all(frags_[which], [&](artdaq::Fragment::value_type i){e.push_back(i);});

  frags_[which].swap(e);
  artdaq::RawFragmentHeader* fh = e.fragmentHeader();
  int event_id = fh->event_id_;
  int from = fh->fragment_id_;
  // make sure the event buffer is big enough
  if (frags_[which].size() < static_cast<size_t>(fragment_words_))
  { frags_[which].resize(fragment_words_); }
  rm.woke(event_id, which);
  Debug << "recv: " << rank_ << " id=" << event_id << " from="
        << from << " which=" << which << flusher;
  // repost the request that was complete, from the same sender
  // This makes the buffer we've just received data on available to receive new data.
  /* rc = */ MPI_Irecv(&(frags_[which])[0],
                       (fragment_words_ * sizeof(artdaq::RawDataType)),
                       MPI_BYTE, from, MY_TAG, MPI_COMM_WORLD, &reqs_[which]);
  // printError(rc,from,stats_[which]);
  rm.post(from);
}

void RHandles::waitAll()
{
  // MPI_Waitall(buffer_count_,&reqs_[0],&stats_[0]);
  // clean up the remaining buffers
  for (int i = 0; i < buffer_count_; ++i) {
    MPI_Cancel(&reqs_[i]);
    MPI_Wait(&reqs_[i], &stats_[i]);
  }
}
