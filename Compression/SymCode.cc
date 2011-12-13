
#include <fstream>
#include <algorithm>
#include <iterator>

#include "SymCode.hh"
#include "Properties.hh"

using namespace std;

void readTable(const char* fname, SymTable& out)
{
  ifstream ifs(fname);
  out.clear();
  out.resize(Properties::count_max());

  // copy(istream_iterator<TableEntry>(ifs),istream_iterator<TableEntry>(),
  //  back_inserter(from_file));

  while(1)
    {
      SymCode te;
      ifs >> te;
      if(ifs.eof() || ifs.bad()) break;
      out[te.sym_] = te;
    }
}

void writeTable(const char* fname, SymTable const& in)
{
  ofstream ofs(fname);
  copy(in.begin(),in.end(),ostream_iterator<SymCode>(ofs,"\n"));
}

void reverseCodes(SymTable& out)
{
  // reverse the bits
  for(auto cur=out.begin(),end=out.end();cur!=end;++cur)
    {
      reg_type in_reg  = cur->code_;
      reg_type out_reg = 0;
      for(reg_type i=0;i<cur->bit_count_;++i) { out_reg<<=1; out_reg|=(in_reg>>i)&0x01; }
      cur->code_ = out_reg;
    }
}
