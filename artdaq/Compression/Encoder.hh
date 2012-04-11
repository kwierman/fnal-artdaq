#ifndef artdaq_Compression_Encoder_hh
#define artdaq_Compression_Encoder_hh

#include <istream>

#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymTable.hh"

namespace ds50 {
  class Encoder;
}

class ds50::Encoder
{
public:
  explicit Encoder(SymTable const&);

  // returns the number of bits in the out buffer
  reg_type operator()(ADCCountVec const& in, DataVec& out);
  reg_type operator()(adc_type const* beg, adc_type const* last, DataVec& out);

private:
  SymTable syms_;
};

#endif /* artdaq_Compression_Encoder_hh */
