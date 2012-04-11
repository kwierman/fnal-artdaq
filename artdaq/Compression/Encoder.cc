
#include <algorithm>
#include <iostream>

#include "artdaq/Compression/Encoder.hh"

using namespace std;
using namespace ds50;

namespace {
  constexpr auto bits_per_word = sizeof(reg_type) * sizeof(char);

  struct Accum {
  public:
    Accum(DataVec & out, SymTable const & syms):
      syms_(syms), max_(out.size()), curr_word_(&out[0]), curr_pos_(0), total_(0)
    { fill(out.begin(), out.end(), 0); }

    void put(ADCCountVec::value_type const & value);
    reg_type totalBits() const { return total_; }

  private:
    SymTable const & syms_;
    size_t max_;
    reg_type * curr_word_;
    reg_type curr_pos_;
    reg_type total_;
  };

  void Accum::put(ADCCountVec::value_type const & val)
  {
    SymTable::value_type const & te = syms_[val];
    //cout << "curr_pos=" << curr_pos_ << " bit_count=" << te.bit_count_ << "\n";
    *curr_word_ |= (te.code_ << curr_pos_);
    curr_pos_ += te.bit_count_;
    total_ += te.bit_count_;
    if (curr_pos_ >= bits_per_word) {
      //cout << "curr=" << curr_pos_ << " bit_per=" << bits_per_word << "\n";
      ++curr_word_;
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
  // return (*this)(in.begin(),in.end(),out);
}

reg_type Encoder::operator()(adc_type const * beg, adc_type const * end, DataVec & out)
{
  Accum a(out, syms_);
  for_each(beg, end, [&](adc_type const & v) { a.put(v); });
  return a.totalBits();
}

