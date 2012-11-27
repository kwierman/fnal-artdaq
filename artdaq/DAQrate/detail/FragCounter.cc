#include "artdaq/DAQrate/detail/FragCounter.hh"

#include "art/Utilities/Exception.h"

size_t
artdaq::detail::FragCounter::
computedSlot_(size_t slot) const
{
  if (slot < offset_) {
    throw art::Exception(art::errors::LogicError, "FragCounter: ")
      << "Specified slot " << slot
      << " inconsistent with specified offset "
      << offset_;
  }
  return slot - offset_;
}
