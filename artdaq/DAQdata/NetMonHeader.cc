#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"
#include "artdaq/DAQdata/NetMonHeader.hh"

using artdaq::detail::RawFragmentHeader;

artdaq::NetMonHeader::type_t const artdaq::NetMonHeader::EventDataFragmentType =
  detail::RawFragmentHeader::FIRST_USER_TYPE;
artdaq::NetMonHeader::type_t const artdaq::NetMonHeader::InitDataFragmentType =
  detail::RawFragmentHeader::FIRST_USER_TYPE + 1;
artdaq::NetMonHeader::type_t const artdaq::NetMonHeader::RunDataFragmentType =
  detail::RawFragmentHeader::FIRST_USER_TYPE + 2;
artdaq::NetMonHeader::type_t const artdaq::NetMonHeader::SubRunDataFragmentType =
  detail::RawFragmentHeader::FIRST_USER_TYPE + 3;
artdaq::NetMonHeader::type_t const artdaq::NetMonHeader::ShutdownFragmentType =
  detail::RawFragmentHeader::FIRST_USER_TYPE + 4;
