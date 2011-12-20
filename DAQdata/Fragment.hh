#ifndef DAQ_FRAGMENT_HH
#define DAQ_FRAGMENT_HH

#include <vector>
#include <stdint.h>


namespace artdaq
{
  typedef uint32_t RawDataType;
  typedef std::vector<RawDataType> Fragment;
  typedef std::vector<uint64_t> CompressedFragPart;
  typedef std::vector<CompressedFragPart> CompressedFragParts;

  struct DarkSideHeaderOverlay
  {
    unsigned long event_size_ : 28;
    unsigned long junk1 : 4;
    unsigned long channel_mask_ : 8;
    unsigned long pattern_ : 16;
    unsigned long junk2 : 3;
    unsigned long board_id_ : 5;
    unsigned long event_counter_ : 24;
    unsigned long reserved_ : 8;
    unsigned long trigger_time_tag_ : 32;
  };

  struct DarkSideHeader
  {
    unsigned long word0;
    unsigned long word1;
  };

  struct CompressedBoard
  {
    DarkSideHeader header_;
    CompressedFragParts parts_;
  };

  typedef std::vector<CompressedBoard> CompressedRawEvent;
}
#endif
