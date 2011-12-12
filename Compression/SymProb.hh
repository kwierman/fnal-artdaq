#ifndef COMP_SYMPROB_H
#define COMP_SYMPROB_H

/*
  jbk - old note from code for incr():
  "this is not the correct sym probability calculation"
*/

#include <ostream>
#include "Properties.hh"

struct SymProb
{
  SymProb():sym_(0),count_(0) { }
  explicit SymProb(unsigned int sym):sym_(sym),count_(0) { }
  
  unsigned int sym_;
  unsigned long count_;
  
  void incr() { ++count_; }
  bool operator<(SymProb const& other) const { return this->count_ > other.count_; }
};

/* for testing */
inline std::ostream& operator<<(std::ostream& ost, SymProb const& s)
{
  ost << "(" << s.sym_ << "," << s.count_ << ")";
  return ost;
}

// --------------- 

typedef std::vector<SymProb> SymsVec;

void calculateProbs(ADCCountVec const& data_in, SymsVec& prob_table_out);

#endif
