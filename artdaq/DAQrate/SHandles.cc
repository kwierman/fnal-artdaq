
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

int artdaq::SHandles::calcDest(Fragment::sequence_id_t sequence_id) const
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

size_t
artdaq::SHandles::
sendFragment(Fragment && frag)
{
  // Precondition: Fragment must be complete and consistent (including
  // header information).
  if (frag.type() == Fragment::type_t::END_OF_DATA) {
    throw cet::exception("LogicError")
      << "EOD fragments should not be sent on as received: "
      << "use sendEODFrag() instead.";
  }
  int dest = calcDest(frag.sequenceID());
  sendFragTo(std::move(frag), dest);
  return dest;
}


void
artdaq::SHandles::
sendEODFrag(int dest, size_t nFragments)
{
  sendFragTo(Fragment::eodFrag(nFragments), dest);
}

void artdaq::SHandles::waitAll()
{
  MPI_Waitall(buffer_count_, &reqs_[0], &stats_[0]);
}

void
artdaq::SHandles::
sendFragTo(Fragment && frag, int dest)
{
  if (frag.size() > max_initial_send_words_) {
    throw cet::exception("Unimplemented")
      << "Current unable to deal with overlarge fragments (size > "
      << max_initial_send_words_
      << " words).";
  }
  int buffer_idx = findAvailable();
  Fragment & curfrag = payload_[buffer_idx];
  curfrag = std::move(frag);
  SendMeas sm;
  sm.found(curfrag.sequenceID(), buffer_idx, dest);
  Debug << "send COMPLETE: "
        << " buffer_idx=" << buffer_idx
        << " send_size=" << curfrag.size()
        << " dest=" << dest
        << " sequenceID=" << curfrag.sequenceID()
        << " fragID=" << curfrag.fragmentID()
        << flusher;
  MPI_Isend(&*curfrag.headerBegin(),
            curfrag.size() * sizeof(Fragment::value_type),
            MPI_BYTE,
            dest,
            MPITag::FINAL,
            MPI_COMM_WORLD,
            &reqs_[buffer_idx]);
}
