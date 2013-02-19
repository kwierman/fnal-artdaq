#ifndef ds50daq_DAQ_Config_hh
#define ds50daq_DAQ_Config_hh

#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"

namespace ds50
{
  namespace Config
  {
    enum TaskType : int {FragmentReceiverTask=1, EventBuilderTask=2, AggregatorTask=3};

    static const artdaq::detail::RawFragmentHeader::type_t V172X_FRAGMENT_TYPE = 17;
    static const artdaq::detail::RawFragmentHeader::type_t V1495_FRAGMENT_TYPE = 18;
  }
}

#endif
