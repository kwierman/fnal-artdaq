
#include <algorithm>
// next ones for debugging
#include <iterator>
#include <iostream>

#include "SymProb.hh"

using namespace std;

void calculateProbs(ADCCountVec const & d, SymsVec & out, size_t countmax)
{
  unsigned int symnum = 0;
  out.clear();
  out.reserve(Properties::count_max());
  // generate one slot for possible symbols
  generate_n(back_inserter(out), countmax,
  [&]() { return SymProb(symnum++); });
  for_each(d.cbegin(), d.cend(),
  [&](ADCCountVec::value_type v) { out[v].incr(); });
  for_each(out.begin(), out.end(),
  [](SymsVec::value_type & v) { if (v.count == 0) { v.count = 1; } });
  // must leave zero count entries in
  sort(out.begin(), out.end()); // descending
  // copy(out.begin(),out.end(),ostream_iterator<SymProb>(cout,"\n"));
}

