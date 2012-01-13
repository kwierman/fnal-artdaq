
#include "SHandles.hh"
#include "Perf.hh"
#include "Debug.hh"
#include "Utils.hh"
#include "DAQdata/RawData.hh"

#define MY_TAG 2

// size_ = number of buffers
// fragment_size_ = number of longs in a fragment of an event
//
// be careful about the event/fragment size - hardwired to sizeof(long)
// and should be determined from the FragmentPool

SHandles::SHandles(Config const & conf):
  buffer_count_(conf.source_buffer_count_),
  fragment_words_(conf.fragment_words_),
  dest_count_(conf.destCount()),
  dest_start_(conf.destStart()), // starts after last source
  rank_(conf.rank_),
  pos_(),
  reqs_(buffer_count_, MPI_REQUEST_NULL),
  stats_(buffer_count_),
  flags_(buffer_count_),
  frags_(buffer_count_),
  is_direct_(conf.type_ == Config::TaskDetector),
  friend_(conf.getDestFriend())
{
}

int SHandles::dest(long event_id) const
{
  // sinks are [offset,total_nodes)
  return is_direct_ ?
         (friend_) :
         (int)(event_id % dest_count_ + dest_start_);
}

int SHandles::findAvailable()
{
  int use_me = 0;
  do {
    use_me = pos_;
    MPI_Test(&reqs_[use_me], &flags_[use_me], &stats_[use_me]);
    pos_ = (pos_ + 1) % buffer_count_;
  }
  while (!flags_[use_me]);
  // pos_ is pointing at the next slot to check
  // use_me is pointing at the slot to use
  return use_me;
}

void SHandles::sendEvent(Data & e)
{
  SendMeas sm;
  int use_me = findAvailable();
  frags_[use_me].swap(e);
  artdaq::RawFragmentHeader* h = (artdaq::RawFragmentHeader*)&frags_[use_me][0];
  int event_id = h->event_id_;
  h->fragment_id_ = rank_;
  int event_size = frags_[use_me].size();
  sm.found(event_id, use_me, dest(event_id));
  Debug << "send: " << rank_ << " id=" << event_id << " size=" << event_size
        << " idx=" << use_me << " dest=" << dest(event_id) << flusher;
  MPI_Isend(&(frags_[use_me])[0], (fragment_words_ * sizeof(artdaq::RawDataType)),
            MPI_BYTE, dest(event_id),
            MY_TAG, MPI_COMM_WORLD, &reqs_[use_me]);
#if 0
  writeData(rank_, (const char*) & (frags_[use_me])[0], frags_[use_me].size()*sizeof(Data::value_type));
#endif
}

void SHandles::waitAll()
{
  MPI_Waitall(buffer_count_, &reqs_[0], &stats_[0]);
}

