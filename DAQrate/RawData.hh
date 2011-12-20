#ifndef RAWDATA_HHH
#define RAWDATA_HHH

#include <memory>
#include <vector>
#include <stdint.h>

typedef uint32_t RawDataType;

struct RawEventHeader
{
public:
  RawDataType word_count_;
  RawDataType run_id_;
  RawDataType subrun_id_;
  RawDataType event_id_;
};

struct RawEvent
{
  typedef std::vector<RawDataType> Fragment;
  typedef std::shared_ptr<Fragment> FragmentPtr;

public:
  RawEventHeader header_;

  std::vector<FragmentPtr> fragment_list_;
};

struct RawFragmentHeader
{
public:
  RawDataType word_count_;
  RawDataType event_id_;
  RawDataType fragment_id_;
};

#endif
