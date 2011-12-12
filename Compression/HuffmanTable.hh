#ifndef COMP_TABLE
#define COMP_TABLE

#include <memory>
#include <vector>
#include <list>
#include <istream>
#include <string>

#include "Tree.hh"
#include "Properties.hh"
#include "SymCode.hh"
#include "SymProb.hh"

class HuffmanTable
{
public:
  typedef std::unique_ptr<Node> Node_ptr;
  typedef std::list<Node*> HeadList;
  typedef std::vector<Node_ptr>NodeVec;

  struct ItPair
  {
    typedef HeadList::iterator It;
    It low_;
    It high_;
    
    It left() { return high_; }
    It right() { return low_; }
    
    ItPair():low_(),high_() { }
    ItPair(It higher, It lower):low_(lower),high_(higher) { }
  };

  typedef ItPair (*Algo)(HeadList&);

  HuffmanTable(std::istream& training_set);
  HuffmanTable(ADCCountVec const& training_set);

  void extractTable(SymTable& out) const;
  void writeTable(std::string const& filename) const;

private:
  void initNodes(SymsVec const&);
  void initHeads();
  void constructTree();
  void makeTable(ADCCountVec const&);

  NodeVec nodes_;
  HeadList heads_;
};

#endif
