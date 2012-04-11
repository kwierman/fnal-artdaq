#include "artdaq/Compression/SymTable.hh"
#include "artdaq/Compression/Properties.hh"

#include <fstream>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace ds50;

void ds50::readTable(const char* fname, SymTable & out, size_t countmax)
{
  std::ifstream ifs(fname);
  out.clear();
  out.resize(countmax);
  // copy(istream_iterator<TableEntry>(ifs),istream_iterator<TableEntry>(),
  //  back_inserter(from_file));
  while (1) {
    SymCode te;
    ifs >> te;
    if (ifs.eof() || ifs.bad()) { break; }
    out[te.sym_] = te;
  }
}

void ds50::writeTable(const char* fname, SymTable const & in)
{
  ofstream ofs(fname);
  copy(in.cbegin(), in.cend(), ostream_iterator<SymCode>(ofs, "\n"));
}

void ds50::reverseCodes(SymTable & out)
{
  // reverse the bits
  for (auto cur = out.begin(), end = out.end(); cur != end; ++cur) {
    reg_type in_reg  = cur->code_;
    reg_type out_reg = 0;
    for (reg_type i = 0; i < cur->bit_count_; ++i) { out_reg <<= 1; out_reg |= (in_reg >> i) & 0x01; }
    cur->code_ = out_reg;
  }
}
