#ifndef COMP_ENCODER_H
#define COMP_ENCODER_H

#include <istream>

#include "Properties.hh"
#include "SymCode.hh"

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

class Encoder
{
public:
  explicit Encoder(SymTable const&);

  // returns the number of bits in the out buffer
  reg_type operator()(ADCCountVec const& in, DataVec& out);

private:
  SymTable const& syms_;
};

#endif
