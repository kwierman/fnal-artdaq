#ifndef artdaq_Compression_BlockReader_hh
#define artdaq_Compression_BlockReader_hh

#include "Properties.hh"
#include <iosfwd>


class BlockReader
{
public:
  explicit BlockReader(std::istream&);

  // number of words read and placed into out is returned
  reg_type next(ADCCountVec& out);

private:
  std::istream* ist_;
  ADCCountVec buffer_;
};

#endif
