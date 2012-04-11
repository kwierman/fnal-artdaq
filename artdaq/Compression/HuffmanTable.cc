
#include <fstream>
#include <iostream>
#include <algorithm>

#include "artdaq/Compression/HuffmanTable.hh"
#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymProb.hh"

using namespace std;
using namespace ds50;

namespace {
  void readTrainingSet(istream & ifs, ADCCountVec & out, size_t max_samples)
  {
    bool forever = max_samples == 0 ? true : false;
    const size_t sz = sizeof(adc_type);
    while (forever || max_samples > 0) {
      adc_type data;
      ifs.read((char *)&data, sz);
      if (ifs.eof()) { break; }
      out.push_back(data);
      --max_samples;
    }
  }

  // ---- combiner algorithms -------

  HuffmanTable::ItPair LowLow(HuffmanTable::HeadList & heads)
  {
    auto lower = --heads.rbegin().base();
    auto higher = lower;
    --higher;
    HuffmanTable::ItPair p(higher, lower);
    return p;
  }

  HuffmanTable::ItPair HighHigh(HuffmanTable::HeadList & heads)
  {
    auto higher = heads.begin();
    auto lower  = higher; ++lower;
    HuffmanTable::ItPair p(higher, lower);
    return p;
  }

  HuffmanTable::ItPair HighLow(HuffmanTable::HeadList & heads)
  {
    HuffmanTable::ItPair p(heads.begin(), --heads.rbegin().base());
    return p;
  }

}

HuffmanTable::HuffmanTable(std::istream & ifs,
                           size_t countmax, size_t max_samples)
{
  ADCCountVec t;
  readTrainingSet(ifs, t, max_samples);
  makeTable(t, countmax);
}

HuffmanTable::HuffmanTable(ADCCountVec const & t, size_t countmax)
{
  makeTable(t, countmax);
}

void HuffmanTable::makeTable(ADCCountVec const & adcs, size_t countmax)
{
  SymsVec sv;
  calculateProbs(adcs, sv, countmax);
  initNodes(sv);
  initHeads();
  constructTree();
}

void HuffmanTable::initNodes(SymsVec const & syms)
{
  nodes_.clear();
  transform(syms.begin(), syms.end(), back_inserter(nodes_),
            [&](SymsVec::value_type const & e)
  { return Node_ptr(new Leaf(e.count, e.sym)); });
}

void HuffmanTable::initHeads()
{
  heads_.clear();
  transform(nodes_.begin(), nodes_.end(), back_inserter(heads_),
  [&](NodeVec::value_type const & e) { return e.get(); });
}

void HuffmanTable::constructTree()
{
  while (++heads_.begin() != heads_.end()) {
    ItPair p = LowLow(heads_);
    nodes_.push_back(Node_ptr(new Branch(*(p.left()), *(p.right()))));
    heads_.erase((p.low_));
    heads_.erase((p.high_));
    // need to be put into the right sorted order spot
    auto pos = find_if(heads_.rbegin(), heads_.rend(),
    [&](HeadList::value_type & v) {return nodes_.back()->count() <= v->count();});
    heads_.insert(pos.base(), nodes_.back().get());
  }
}

void HuffmanTable::extractTable(SymTable & out) const
{
  SymCode::data_type reg = 0;
  heads_.front()->traverse(0, reg, out);
}

void HuffmanTable::writeTable(string const & filename) const
{
  SymTable tab;
  extractTable(tab);
  ::writeTable(filename.c_str(), tab);
}

void HuffmanTable::writeTableReversed(string const & filename) const
{
  SymTable tab;
  extractTable(tab);
  ::reverseCodes(tab);
  ::writeTable(filename.c_str(), tab);
}

