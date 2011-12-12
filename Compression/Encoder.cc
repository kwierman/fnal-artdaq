
#include <algorithm>
#include <iostream>

#include "Encoder.hh"

using namespace std;

namespace 
{
  constexpr auto bits_per_word = sizeof(reg_type) * 8;

  struct Accum
  {
  public:
    Accum(DataVec& out, SymTable const& syms):
      syms_(syms),max_(out.size()),curr_word_(&out[0]),curr_pos_(0),total_(0)
    { }
 
    void put(ADCCountVec::value_type const& value);
    reg_type totalBits() const { return total_; }

  private:
    SymTable const& syms_;
    size_t max_;
    reg_type* curr_word_;
    reg_type curr_pos_;
    reg_type total_;
  };

  void Accum::put(ADCCountVec::value_type const& val)
  {
    SymTable::value_type const& te = syms_[val];
    //cout << "curr_pos=" << curr_pos_ << " bit_count=" << te.bit_count_ << "\n";

    *curr_word_ |= te.code_ << curr_pos_;
    curr_pos_ += te.bit_count_;
    total_ += te.bit_count_;

    if( curr_pos_ >= bits_per_word )
      {
	//cout << "curr=" << curr_pos_ << " bit_per=" << bits_per_word << "\n";
	++curr_word_;
	curr_pos_ = curr_pos_ - bits_per_word; // leftovers
	*curr_word_ = te.code_ >> (te.bit_count_ - curr_pos_);
      }
  }
}

// -----------------

Encoder::Encoder(SymTable const& syms):
  syms_(syms)
{
}

reg_type Encoder::operator()(ADCCountVec const& in, DataVec& out)
{
  Accum a(out,syms_);
  for_each(in.begin(),in.end(),[&](ADCCountVec::value_type const& v) { a.put(v); });
  return a.totalBits();
}

// -----------------

BlockReader::BlockReader(std::istream& ist):ist_(&ist),buffer_(chunk_size_counts)
{
}

reg_type BlockReader::next(ADCCountVec& out)
{
  // this entire implementation is not good
  if(buffer_.size()<chunk_size_counts) buffer_.resize(chunk_size_counts);

  size_t bytes_read = ist_->readsome((char*)&buffer_[0],chunk_size_bytes);
  reg_type rc = (ist_->eof() || bytes_read==0) ? 0 : bytes_read/sizeof(ADCCountVec::value_type);

#if 0
  cout << "read bytes=" << bytes_read 
       << " chunk_size_counts=" << chunk_size_counts 
       << " rc=" << rc
       << "\n";
#endif

  if(rc)
    {
      out.swap(buffer_);
      out.resize(rc);
    }

  return rc;
}
