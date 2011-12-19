#ifndef RAWDATA_HHH
#define RAWDATA_HHH

#include <boost/shared_ptr.hpp>
#include <vector>
#include <stdint.h>

typedef uint32_t RawDataType;

struct RawEvent
{
  typedef std::vector<RawDataType> Fragment;
  typedef boost::shared_ptr<Fragment> FragmentPtr;

public:
  RawDataType size_;
  RawDataType run_id_;
  RawDataType subrun_id_;
  RawDataType event_id_;

  std::vector<FragmentPtr> fragment_list_;
};

struct RawDataFragment
{
public:
  RawDataType size_;
  RawDataType event_id_;
  RawDataType fragment_id_;
};

#endif
