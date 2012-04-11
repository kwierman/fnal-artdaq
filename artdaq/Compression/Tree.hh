#ifndef artdaq_Compression_Tree_hh
#define artdaq_Compression_Tree_hh

#include <cassert>
#include <ostream>

#include "artdaq/Compression/SymTable.hh"

namespace ds50 {
  class Node;
  class Leaf;
  class Branch;

  std::ostream & operator<<(std::ostream & ost, Node const & n);
}

class ds50::Node {
public:
  typedef unsigned long data_type;

  virtual ~Node() { }
  virtual data_type count() const = 0;
  virtual void traverse(data_type bit_count, data_type bit_register, SymTable & store) const = 0;
  virtual void print(std::ostream & ost) const = 0;
};

class ds50::Leaf : public ds50::Node {
public:
  Leaf(): count_(0), sym_(0) { }
  // We don't need the +1 on cnt, as long as we never add to the alphabet
  // a symbol we say can not appear (i.e. one with a frequency count of 0).
  // We need to make sure the frequency table we generate contains no elements
  // with zero frequency. It is probably safe to replace any 0 frequencies
  // by 1s, since this is making a small fractional change in the non-
  // negligible probabilities.
  //Leaf(data_type cnt, data_type sym):count_(cnt+1),sym_(sym)
  Leaf(data_type cnt, data_type sym): count_(cnt), sym_(sym)
  { assert(cnt != 0); }

  virtual ~Leaf() { }
  virtual data_type count() const { return count_; }
  virtual void traverse(data_type bit_count, data_type bit_register, SymTable & store) const {
    data_type reg = bit_register;
    store.push_back(SymCode(sym_, reg, bit_count));
  }

  virtual void print(std::ostream & ost) const
  //{ ost << "(" << sym_ << "," << count_ << ")"; }
  {
    ost << "L ("
        << this
        << ") ["
        << count_
        << "] "
        << sym_
        << '\n';
  }

private:
  data_type count_;
  data_type sym_;
};

inline std::ostream & ds50::operator<<(std::ostream & ost, Node const & n)
{
  n.print(ost);
  return ost;
}

class ds50::Branch : public ds50::Node {
public:
  Branch(): count_(0), left_(0), right_(0) { }
  Branch(Node * left, Node * right): count_(left->count() + right->count()), left_(left), right_(right)
  { }

  virtual ~Branch() { }
  virtual data_type count() const { return count_; }
  virtual void traverse(data_type bit_count, data_type bit_register, SymTable & store) const {
    left_->traverse(bit_count + 1, (bit_register << 1), store);
    right_->traverse(bit_count + 1, (bit_register << 1) | 0x1, store);
  }

  virtual void print(std::ostream & ost) const {
    ost << "B ("
        <<  this
        << ") ["
        << count_
        << "] "
        << left_
        << " "
        << right_
        << "]\n";
    // ost << "B " << count_ << "(\n";
    left_->print(ost);
    right_->print(ost);
    //ost << "\n)";
  }

private:
  data_type count_;
  Node * left_;
  Node * right_;
};

// ---------------



#endif /* artdaq_Compression_Tree_hh */
