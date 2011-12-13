#ifndef COMP_DECODER_H
#define COMP_DECODER_H

#include <vector>

#include "Properties.hh"
#include "SymCode.hh"

constexpr auto neg_one = ~(0ul);

class Decoder
{
public:
  Decoder(SymTable const&);

  reg_type operator()(reg_type bit_count, DataVec const& in, ADCCountVec& out);

  void printTable(std::ostream& ost) const;

private:

  void buildTable();

  size_t addNode()
  {
    table_.push_back(neg_one);
    table_.push_back(neg_one);
    size_t rc = last_;
    last_ += 2;
    return rc;
  }

  SymTable const& syms_;
  DataVec table_;
  size_t head_;
  size_t last_;
};

#endif
