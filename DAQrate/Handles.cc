
#include "Handles.hh"
#include "Perf.hh"
#include "Debug.hh"

#define MY_TAG 2

// size_ = number of buffers
// fragment_size_ = number of longs in a fragment of an event
//
// be careful about the event/fragment size - hardwired to sizeof(long)
// and should be determined from the EventPool

Handles::Handles(Config const& conf):
  size_(conf.is_sink_?conf.sink_buffer_count_:conf.source_buffer_count_),
  fragment_size_(conf.packet_size_ / sizeof(Data::value_type)),
  sinks_(conf.sinks_),
  reqs_(size_,MPI_REQUEST_NULL),
  stats_(size_),
  flags_(size_),
  events_(size_),
  offset_(conf.offset_),
  rank_(conf.rank_),
  pos_()
{
}

int Handles::dest(long event_id) const
{
  // sinks are [offset,total_nodes)
  return (int)(event_id % sinks_ + offset_);
}

void Handles::waitAll()
{
  MPI_Waitall(size_,&reqs_[0],&stats_[0]);
}

void Handles::cleanup()
{
  // clean up the remaining buffers
  for(int i=0;i<size_;++i)
    {
      MPI_Cancel(&reqs_[i]);
      MPI_Wait(&reqs_[i],&stats_[i]);
    }
}
