#ifndef COMP_TREE_H
#define COMP_TREE_H

#include <ostream>

#include "SymCode.hh"

class Node
{
public:
  typedef unsigned long data_type;

  virtual ~Node() { }
  virtual data_type count() const = 0;
  virtual void traverse(data_type bit_count, data_type bit_register, SymTable& store) const = 0;
  virtual void print(std::ostream& ost) const = 0;
};

class Leaf : public Node
{
public:
  Leaf():count_(0),sym_(0) { }
  Leaf(data_type cnt, data_type sym):count_(cnt+1),sym_(sym) 
  { }

  virtual ~Leaf() { }
  virtual data_type count() const { return count_; }
  virtual void traverse(data_type bit_count, data_type bit_register, SymTable& store) const
  {
    // reverse the bits in the register
    data_type reg = 0;
    for(int i=0;i<bit_count;++i) { reg<<=1; reg|=(bit_register>>i)&0x01; }
    store.push_back( SymCode(sym_,reg,bit_count) );

    // std::cout << "reverse " << (void*)reg << " " << (void*)bit_register << " " << bit_count << "\n";
  }

  virtual void print(std::ostream& ost) const
  { ost << "(" << sym_ << "," << count_ << ")"; }

private:
  data_type count_;
  data_type sym_;
};

inline std::ostream& operator<<(std::ostream& ost, Node const& n)
{
  n.print(ost);
  return ost;
}

class Branch : public Node
{
public:
  Branch(): count_(0),left_(0),right_(0) { }
  Branch(Node* left, Node* right):count_(left->count()+right->count()),left_(left),right_(right)
  { }

  virtual ~Branch() { }
  virtual data_type count() const { return count_; }
  virtual void traverse(data_type bit_count, data_type bit_register, SymTable& store) const
  {
    left_->traverse(bit_count+1,(bit_register<<1),store);
    right_->traverse(bit_count+1,(bit_register<<1)|0x1,store);
  }

  virtual void print(std::ostream& ost) const
  { ost << "B " << count_; }

private:
  data_type count_;
  Node* left_;
  Node* right_;
};

// ---------------



#endif
