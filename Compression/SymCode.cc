
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
