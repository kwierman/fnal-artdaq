
#include "artdaq/Compression/Encoder.hh"
#include "cetlib/exception.h"

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;
using namespace ds50;

namespace {
  constexpr auto bits_per_word = sizeof(reg_type) * 8;

  struct Accum {
  public:
    Accum(DataVec & out, SymTable const & syms);

    void put(ADCCountVec::value_type const & value);
    reg_type totalBits() const { return total_; }

  private:
    SymTable const & syms_;
    reg_type * curr_word_;
    reg_type * buf_end_;
    reg_type curr_pos_;
    reg_type total_;
  };

  Accum::Accum(DataVec & out, SymTable const & syms)
    :
    syms_(syms),
    curr_word_(&*out.begin()),
    buf_end_(&*out.end()),
    curr_pos_(0),
    total_(0)
  {
    if (curr_word_ == buf_end_) {
      throw cet::exception("CompressionBufferSize")
          << "Provided buffer has zero size!";
    }
    fill(out.begin(), out.end(), 0);
  }

  void Accum::put(ADCCountVec::value_type const & val)
  {
    SymTable::value_type const & te = syms_[val];
    *curr_word_ |= (te.code_ << curr_pos_);
    curr_pos_ += te.bit_count_;
    total_ += te.bit_count_;
    if (curr_pos_ >= bits_per_word) {
      if (++curr_word_ == buf_end_) {
        throw cet::exception("CompressionBufferSize")
            << "Size of compression buffer exceeded.";
      }
      curr_pos_ = curr_pos_ - bits_per_word; // leftovers
      *curr_word_ = te.code_ >> (te.bit_count_ - curr_pos_);
    }
  }
}

// -----------------

Encoder::Encoder(SymTable const & syms):
  syms_(syms)
{
  reverseCodes(syms_);
}

reg_type Encoder::operator()(ADCCountVec const & in, DataVec & out)
{
  return (*this)(&in[0], &in[in.size()] , out);
}

reg_type Encoder::operator()(adc_type const * beg, adc_type const * end, DataVec & out)
{
  Accum a(out, syms_);
  for_each(beg, end, [&](adc_type const & v) { a.put(v); });
  return a.totalBits();
}

