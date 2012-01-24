#ifndef RAWDATA_HHH
#define RAWDATA_HHH

#include "Fragment.hh"

#include <memory>
#include <vector>

namespace artdaq
{

  /*
    This header is our own independent header definition
  */

  struct RawEventHeader
  {
    RawDataType word_count_;
    RawDataType run_id_;
    RawDataType subrun_id_;
    RawDataType event_id_;
  };

  /*
    This header is in the front of each fragment
  */

  struct RawFragmentHeader
  {
    RawDataType word_count_;
    RawDataType event_id_;
    RawDataType fragment_id_;
  };

  /*
    Each event has our header plus one or more raw data fragments
  */

  struct RawEvent
  {

    RawEventHeader header_;
    Fragments      fragments_;
  };

}
#endif
