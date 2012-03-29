
#include "artdaq/DAQrate/SHandles.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQrate/Debug.hh"
#include "artdaq/DAQrate/MPITag.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "cetlib/exception.h"

#include <algorithm>

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

int artdaq::SHandles::dest(Fragment::sequence_id_t sequence_id) const
{
  // Works if dest_count_ == 1
  return (int)(sequence_id % dest_count_ + dest_start_);
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

void artdaq::SHandles::sendFragment(Fragment && frag)
{
  // Precondition: Fragment must be complete and consistent (including
  // header information).
  SendMeas sm;
  int use_me = findAvailable();
  Fragment & curfrag = payload_[use_me];
  curfrag = std::move(frag);
  Fragment::sequence_id_t sequence_id = curfrag.sequenceID();
  int const mpi_to = dest(sequence_id);
  int rank;
  assert(MPI_Comm_rank(MPI_COMM_WORLD, &rank) == MPI_SUCCESS);
  if (curfrag.size() > max_initial_send_words_) {
    if (buffer_count_ > 1) {
      throw cet::exception("unimplemented")
        << "Current unable to deal with overlarge fragments when "
        << "running with multiple buffers.";
    }
    // Send in two chunks. Receiver will do the right thing as indicated
    // by the data tag on the first chunk and the fragment size
    // information in the header therein.
    Debug << "send partial: " << rank
          << " id=" << sequence_id
          << " size=" << max_initial_send_words_
          << " idx=" << use_me
          << " dest=" << mpi_to
          << " tag=" << MPITag::INCOMPLETE
          << " fragID=" << curfrag.fragmentID()
          << flusher;
    MPI_Isend(&*curfrag.headerBegin(),
              max_initial_send_words_ * sizeof(Fragment::value_type),
              MPI_BYTE,
              mpi_to,
              MPITag::INCOMPLETE,
              MPI_COMM_WORLD,
              &reqs_[use_me]);
    Debug << "send final: " << rank
          << " id=" << sequence_id
          << " size=" << (curfrag.size() - max_initial_send_words_)
          << " idx=" << use_me
          << " dest=" << mpi_to
          << " tag=" << MPITag::FINAL
          << " fragID=" << curfrag.fragmentID()
          << flusher;
    MPI_Isend(&*curfrag.headerBegin() + max_initial_send_words_,
              (curfrag.size() - max_initial_send_words_) *
              sizeof(Fragment::value_type),
              MPI_BYTE,
              mpi_to,
              MPITag::FINAL,
              MPI_COMM_WORLD,
              &reqs_[use_me]);
  }
  else {
    // Send as one chunk.
    Debug << "send complete: " << rank
          << " id=" << sequence_id
          << " size=" << curfrag.size()
          << " idx=" << use_me
          << " dest=" << mpi_to
          << " tag=" << MPITag::FINAL
          << " fragID=" << curfrag.fragmentID()
          << flusher;
    MPI_Isend(&*curfrag.headerBegin(),
              curfrag.size() * sizeof(Fragment::value_type),
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
