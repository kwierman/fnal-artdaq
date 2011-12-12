
#include <algorithm>
// next ones for debugging
#include <iterator>
#include <iostream>

#include "SymProb.hh"

using namespace std;

void calculateProbs(ADCCountVec const& d, SymsVec& out)
{
  unsigned int symnum=0;
  
  out.clear();
  out.reserve(Properties::count_max());

  // generate one slot for possible symbols
  generate_n(back_inserter(out),Properties::count_max(),
	     [&]() { return SymProb(symnum++); });

  for_each(d.begin(),d.end(), 
	   [&] (ADCCountVec::value_type v) { out[v].incr(); });

  // must leave zero count entries in
  sort(out.begin(),out.end()); // descending
  // copy(out.begin(),out.end(),ostream_iterator<SymProb>(cout,"\n"));
}

