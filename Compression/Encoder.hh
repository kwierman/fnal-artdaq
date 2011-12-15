#ifndef COMP_ENCODER_H
#define COMP_ENCODER_H

#include <istream>

#include "Properties.hh"
#include "SymCode.hh"

class Encoder
{
public:
  explicit Encoder(SymTable const&);

  // returns the number of bits in the out buffer
  reg_type operator()(ADCCountVec const& in, DataVec& out);

private:
  SymTable syms_;
};

#endif
