
#include "artdaq/DAQrate/SHandles.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQdata/Debug.hh"
#include "artdaq/DAQrate/MPITag.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "cetlib/exception.h"

#include <algorithm>

artdaq::SHandles::SHandles(size_t buffer_count,
                           uint64_t max_payload_size,
                           size_t dest_count,
                           size_t dest_start)
  :
  buffer_count_(buffer_count),
  max_payload_size_(max_payload_size),
  dest_count_(dest_count),
  dest_start_(dest_start),
  pos_(),
  sent_frag_count_(dest_count, dest_start),
  reqs_(buffer_count_, MPI_REQUEST_NULL),
  stats_(buffer_count_),
  flags_(buffer_count_),
  payload_(buffer_count_)
{
}

artdaq::SHandles::~SHandles()
{
  size_t dest_end = dest_start_ + dest_count_;
  for (size_t dest = dest_start_; dest != dest_end; ++dest) {
    std::ostringstream os;
    os << "dest: " << dest << ", count: " << sent_frag_count_.slotCount(dest) << "\n";
    std::cerr << os.str();
    sendEODFrag(dest, sent_frag_count_.slotCount(dest));
  }
  waitAll();
}

size_t artdaq::SHandles::calcDest(Fragment::sequence_id_t sequence_id) const
{
  // Works if dest_count_ == 1
  return sequence_id % dest_count_ + dest_start_;
}

size_t artdaq::SHandles::findAvailable()
{
  size_t use_me = 0;
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
  size_t dest = calcDest(frag.sequenceID());
  sendFragTo(std::move(frag), dest);
  sent_frag_count_.incSlot(dest);
  return dest;
}


void
artdaq::SHandles::
sendEODFrag(size_t dest, size_t nFragments)
{
  sendFragTo(Fragment::eodFrag(nFragments), dest);
}

void artdaq::SHandles::waitAll()
{
  MPI_Waitall(buffer_count_, &reqs_[0], &stats_[0]);
}

void
artdaq::SHandles::
sendFragTo(Fragment && frag, size_t dest)
{
  if (frag.dataSize() > max_payload_size_) {
    throw cet::exception("Unimplemented")
      << "Currently unable to deal with overlarge fragment payload ("
      << frag.dataSize()
      << " words > "
      << max_payload_size_
      << ").";
  }
  SendMeas sm;
  size_t buffer_idx = findAvailable();
  sm.found(frag.sequenceID(), buffer_idx, dest);
  Fragment & curfrag = payload_[buffer_idx];
  curfrag = std::move(frag);
  MPI_Isend(&*curfrag.headerBegin(),
            curfrag.size() * sizeof(Fragment::value_type),
            MPI_BYTE,
            dest,
            MPITag::FINAL,
            MPI_COMM_WORLD,
            &reqs_[buffer_idx]);
  Debug << "send COMPLETE: "
        << " buffer_idx=" << buffer_idx
        << " send_size=" << curfrag.size()
        << " dest=" << dest
        << " sequenceID=" << curfrag.sequenceID()
        << " fragID=" << curfrag.fragmentID()
        << flusher;
}
