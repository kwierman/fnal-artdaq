#ifndef RAWDATA_HHH
#define RAWDATA_HHH

#include <memory>
#include <vector>

namespace artdaq
{
  typedef uint32_t RawDataType;

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

}
#endif
