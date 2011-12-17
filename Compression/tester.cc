
#include "Properties.hh"
#include "HuffmanTable.hh"
#include "Decoder.hh"

#include <algorithm>
#include <iterator>
#include <iostream>

using namespace std;

inline
void push_n(ADCCountVec & v, ADCCountVec::value_type x, size_t n)
{
  for (size_t i = 0; i < n; ++i) { v.push_back(x); }
}

int main()
{
  //ADCCountVec samples( { 3, 3, 3, 3, 2, 2, 2, 1, 1, 0 });
  ADCCountVec samples;
  push_n(samples, 0, 7);
  push_n(samples, 1, 2);
  push_n(samples, 2, 3);
  push_n(samples, 3, 3);
  push_n(samples, 4, 3);
  push_n(samples, 5, 5);
  push_n(samples, 6, 8);
  push_n(samples, 7, 9);
  SymsVec probs;
  calculateProbs(samples, probs, Properties_t<3>::count_max());
  cout << "Frequency table:\n";
  copy(probs.cbegin(), probs.cend(), ostream_iterator<SymProb>(cout, "\n"));
  cout << "-----\n";
  cout << "Huffman tree:\n";
  HuffmanTable h(samples, Properties_t<3>::count_max());
  cout << h << "\n";
  cout << "-----\n";
  SymTable tab;
  h.extractTable(tab);
  cout << "(original) Symbol table\n";
  copy(tab.cbegin(), tab.cend(), ostream_iterator<SymCode>(cout, "\n"));
  cout << "-----\n";
  Decoder d(tab);
  cout << "(recovered) Symbol table\n";
  d.printTable(cout);
  return 0;
}
