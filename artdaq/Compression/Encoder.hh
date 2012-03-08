#ifndef artdaq_Compression_Encoder_hh
#define artdaq_Compression_Encoder_hh

#include <istream>

#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymTable.hh"

class Encoder
{
public:
  explicit Encoder(SymTable const&);

  // returns the number of bits in the out buffer
  reg_type operator()(ADCCountVec const& in, DataVec& out);

private:
  SymTable syms_;
};

#endif /* artdaq_Compression_Encoder_hh */
