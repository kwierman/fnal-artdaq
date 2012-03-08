
#include <algorithm>
// next ones for debugging
#include <iterator>
#include <iostream>

#include "artdaq/Compression/SymProb.hh"

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
           [&](ADCCountVec::value_type v)
  { if (v < countmax) { out[v].incr(); } });
  // must leave zero count entries in
  for_each(out.begin(), out.end(),
  [](SymsVec::value_type & v) { if (v.count == 0) { v.count = 1; } });
  sort(out.begin(), out.end()); // descending
  // copy(out.begin(),out.end(),ostream_iterator<SymProb>(cout,"\n"));
}

unsigned long calculateRunLengths(ADCCountVec const & d, DataVec & out,
                                  unsigned long & run_median)
{
  // unsigned int symnum = 0;
  out.clear();
  out.reserve(50);
  DataVec runs;
  runs.reserve(1000);
  unsigned long one_bits = 0, total_bits = 0, bit_count = 0;
  for (auto b = d.cbegin(), e = d.cend(); b != e; ++b) {
    auto curr = *b;
    for (size_t i = 0; i < sizeof(ADCCountVec::value_type); ++i) {
      one_bits += (curr & 0x01);
      ++total_bits;
      if ((curr & 0x01)) {
        if (out.size() <= bit_count) { out.resize(bit_count + 1); }
        ++out[bit_count];
        runs.push_back(bit_count);
        bit_count = 0;
      }
      else
      { ++bit_count; }
      curr >>= 1;
    }
  }
  sort(runs.begin(), runs.end()); // ascending
  size_t num_runs = runs.size();
  run_median = (num_runs % 2) == 1 ?
               runs[num_runs / 2] : (runs[num_runs / 2 - 1] + runs[num_runs / 2]) / 2;
  // sort(out.begin(), out.end()); // descending
  return one_bits;
}
