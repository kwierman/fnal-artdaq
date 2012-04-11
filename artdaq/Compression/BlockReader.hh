#ifndef artdaq_Compression_BlockReader_hh
#define artdaq_Compression_BlockReader_hh

#include "artdaq/Compression/Properties.hh"
#include <iosfwd>

namespace ds50 {
  class BlockReader;
}

class ds50::BlockReader {
public:
  explicit BlockReader(std::istream &);

  // number of words read and placed into out is returned
  reg_type next(ADCCountVec & out);

private:
  std::istream * ist_;
  ADCCountVec buffer_;
};

#endif /* artdaq_Compression_BlockReader_hh */
