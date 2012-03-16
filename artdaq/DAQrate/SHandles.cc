
#include "artdaq/DAQrate/SHandles.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQrate/Debug.hh"
#include "artdaq/DAQrate/MPITag.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "artdaq/DAQdata/Fragment.hh"

// size_ = number of buffers
// fragment_size_ = number of longs in a fragment of an event
//
// be careful about the event/fragment size - hardwired to sizeof(long)
// and should be determined from the FragmentPool

artdaq::SHandles::SHandles(size_t buffer_count,
                           uint64_t max_initial_send_words,
                           size_t dest_count,
                           size_t dest_start)
  :
  buffer_count_(buffer_count),
  max_initial_send_words_(max_initial_send_words),
  dest_count_(dest_count),
  dest_start_(dest_start),
  pos_(),
  reqs_(buffer_count_, MPI_REQUEST_NULL),
  stats_(buffer_count_),
  flags_(buffer_count_),
  payload_(buffer_count_)
{
}

int artdaq::SHandles::dest(Fragment::event_id_t event_id) const
{
  // Works if dest_count_ == 1
  return (int)(event_id % dest_count_ + dest_start_);
}

int artdaq::SHandles::findAvailable()
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

void artdaq::SHandles::sendEvent(Fragment & frag)
{
  // Precondition: Fragment must be complete and consistent (including
  // header information).
  SendMeas sm;
  int use_me = findAvailable();
  Fragment & curfrag = payload_[use_me];
  curfrag.swap(frag);
  Fragment::event_id_t event_id = curfrag.eventID();
  int const mpi_to = dest(event_id);
  Debug << "send: " << stats_[use_me].MPI_SOURCE
        << " id=" << event_id
        << " size=" << curfrag.dataSize()
        << " idx=" << use_me
        << " dest=" << dest(event_id)
        << flusher;
  if (!(curfrag.size() > max_initial_send_words_)) {  // Send as one chunk.
    MPI_Isend(&*curfrag.dataBegin(),
              curfrag.dataSize() * sizeof(Fragment::value_type),
              MPI_BYTE,
              mpi_to,
              MPITag::FINAL,
              MPI_COMM_WORLD,
              &reqs_[use_me]);
  }
  else { // Send in two chunks. Receiver will do the right thing as
    // indicated by the data tag on the first chunk and the
    // fragment size information in the header therein.
    MPI_Isend(&*curfrag.dataBegin(),
              max_initial_send_words_ * sizeof(Fragment::value_type),
              MPI_BYTE,
              mpi_to,
              MPITag::INCOMPLETE,
              MPI_COMM_WORLD,
              &reqs_[use_me]);
    MPI_Isend(&*curfrag.dataBegin() + max_initial_send_words_,
              (curfrag.size() - max_initial_send_words_) *
              sizeof(Fragment::value_type),
              MPI_BYTE,
              mpi_to,
              MPITag::FINAL,
              MPI_COMM_WORLD,
              &reqs_[use_me]);
  }
}

void artdaq::SHandles::waitAll()
{
  MPI_Waitall(buffer_count_, &reqs_[0], &stats_[0]);
}

