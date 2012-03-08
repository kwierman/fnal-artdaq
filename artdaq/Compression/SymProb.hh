#ifndef artdaq_Compression_SymProb_hh
#define artdaq_Compression_SymProb_hh

/*
  jbk - old note from code for incr():
  "this is not the correct sym probability calculation"
*/

#include <ostream>
#include "artdaq/Compression/Properties.hh"

struct SymProb
{
  SymProb() : sym(0), count(0) { }
  explicit SymProb(unsigned int s, unsigned long c=0UL) : sym(s),count(c) { }
  
  unsigned int sym;
  unsigned long count;
  
  void incr() { ++count; }
  bool operator<(SymProb const& other) const { return this->count > other.count; }
};

/* for testing */
inline std::ostream& operator<<(std::ostream& ost, SymProb const& s)
{
  ost << "(" << s.sym << "," << s.count << ")";
  return ost;
}

// --------------- 

typedef std::vector<SymProb> SymsVec;

void calculateProbs(ADCCountVec const& data_in, SymsVec& prob_table_out, size_t countmax);

// returns number of one bits in d. and the median in run_median
unsigned long calculateRunLengths(ADCCountVec const& d, DataVec& out, unsigned long& run_median);

#endif /* artdaq_Compression_SymProb_hh */
