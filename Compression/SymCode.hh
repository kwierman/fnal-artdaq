#ifndef COMP_SYMCODE_H
#define COMP_SYMCODE_H

#include <ostream>
#include <istream>
#include <vector>

struct SymCode {
  typedef unsigned long data_type;

  SymCode(): sym_(0), code_(0), bit_count_(0) { }
  SymCode(data_type sym, data_type code, data_type count):
    sym_(sym), code_(code), bit_count_(count) {
  }

  data_type sym_;
  data_type code_;
  data_type bit_count_;

  void printHex(std::ostream & ost) const {
    ost << (void*)sym_  << " " << (void*)code_ << " " << bit_count_;
  }

  void print(std::ostream & ost) const {
    ost << sym_ << " " << code_ << " " << bit_count_;
  }
};

inline std::ostream & operator<<(std::ostream & ost, SymCode const & st)
{
  st.print(ost);
  return ost;
}

inline std::istream & operator>>(std::istream & ist, SymCode & t)
{
  if (ist) { ist >> t.sym_; }
  if (ist) { ist >> t.code_; }
  if (ist) { ist >> t.bit_count_; }
  return ist;
}


#endif
