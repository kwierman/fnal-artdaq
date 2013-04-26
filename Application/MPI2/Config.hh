#ifndef ds50daq_DAQ_Config_hh
#define ds50daq_DAQ_Config_hh

#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"

namespace ds50
{
  namespace Config
  {
    enum TaskType : int {FragmentReceiverTask=1, EventBuilderTask=2, AggregatorTask=3};

    static const artdaq::detail::RawFragmentHeader::type_t MISSED_FRAGMENT_TYPE = 1;
    static const artdaq::detail::RawFragmentHeader::type_t V1495_FRAGMENT_TYPE =  2;
    static const artdaq::detail::RawFragmentHeader::type_t V1720_FRAGMENT_TYPE =  3;
    static const artdaq::detail::RawFragmentHeader::type_t V1724_FRAGMENT_TYPE =  4;
    static const artdaq::detail::RawFragmentHeader::type_t V1190_FRAGMENT_TYPE =  5;
  }
}

#endif
