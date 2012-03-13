#ifndef artdaq_DAQdata_DS50data_hh
#define artdaq_DAQdata_DS50data_hh

#error OBSOLETE HEADER: use DS50Board.hh instead.

namespace artdaq
{
  struct DarkSideHeader
  {
    uint32_t word0;
    uint32_t word1;
  };

  struct CompressedBoard
  {
    DarkSideHeader header_;
    CompressedFragParts parts_;
  };
}

#endif /* artdaq_DAQdata_DS50data_hh */
