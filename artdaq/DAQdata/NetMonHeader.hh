#ifndef artdaq_DAQdata_NetMonHeader_hh
#define artdaq_DAQdata_NetMonNeta_hh

#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"

using artdaq::detail::RawFragmentHeader;

namespace artdaq {
  struct NetMonHeader;
}

struct artdaq::NetMonHeader {
  uint64_t data_length;

  typedef RawFragmentHeader::type_t type_t;

  static type_t const EventDataFragmentType;
  static type_t const InitDataFragmentType;
  static type_t const RunDataFragmentType;
  static type_t const SubRunDataFragmentType;
};

#endif
