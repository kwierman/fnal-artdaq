#ifndef artdaq_DAQdata_RawEvent_hh
#define artdaq_DAQdata_RawEvent_hh

#include "RawData.hh"
#include "Fragments.hh"

namespace artdaq
{

  /*
    Each event has our header plus one or more raw data fragments
  */

  struct RawEvent
  {

    RawEventHeader header_;
    FragmentPtrs   fragments_;
  };

}

#endif
