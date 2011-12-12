
#include <algorithm>
#include <iostream>

#include "Decoder.hh"

using namespace std;

namespace 
{
}

Decoder::Decoder(SymTable const& s):syms_(s),table_(),head_(syms_.size())
{
  table_.reserve(syms_.size() * 10);

  for_each(syms_.begin(),syms_.end(),
	   [&](SymCode const& s) { table_.push_back(s.sym_); });

  last_ = table_.size()-1;
  head_ = addNode();

  buildTable();
}

void Decoder::buildTable()
{
  for(size_t s=0,len=table_.size();s<len;++s)
    {
      reg_type code = syms_[s].code_;

      size_t index = 0;
      size_t curr = head_; // start at top;

      // go through all bits in the code for the symbol
      for(size_t i=0;i<syms_[s].bit_count_-1;++i)
	{
	  index = code&0x01;

	  if(table_[curr+index]==neg_one)
	    {
	      size_t newnode = addNode();
	      table_[curr+index]=newnode;
	    }

	  curr=table_[curr+index];
	  code>>=1;
	}

      index = code&0x01;
      if(table_[curr+index]!=neg_one) 
	{
	  std::cerr << "table[" << (curr) << "]=" << table_[curr] << " last=" << last_ << "\n";
	  throw "bad run!";
	}
      table_[curr+index]=s;
    }
}

reg_type Decoder::operator()(reg_type bit_count, DataVec const& in, ADCCountVec& out)
{
  out.clear();
  ADCCountVec tmp;
  size_t curr = head_;
  reg_type const* pos = &in[0];
  reg_type val = *pos;

  for(reg_type i=0;i<bit_count;++i)
    {
      auto inc = (i%65+1)/65;

      if(inc)
	{
	  ++pos;
	  val=*pos;
	}
      
      curr = table_[curr+(val&0x01)];
      val>>=1;

      if(curr<head_)
	{
	  adc_type adcval = (adc_type)syms_[curr].sym_;
	  tmp.push_back(adcval);
	  curr=head_;
	}
    }

  tmp.swap(out);
  return out.size();
}
