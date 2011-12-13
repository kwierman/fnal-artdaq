
#include "Properties.hh"
#include "HuffmanTable.hh"
#include "Decoder.hh"

#include <algorithm>
#include <iterator>
#include <iostream>

using namespace std;

int main()
{
  ADCCountVec samples ( { 3,3,3,3,2,2,2,1,1,0 } );

  SymsVec probs;
  calculateProbs(samples,probs);
  copy(probs.cbegin(),probs.cend(),ostream_iterator<SymProb>(cout,"\n"));

  cout << "-----\n";

  HuffmanTable h(samples);
  cout << h << "\n";

  SymTable tab;
  h.extractTable(tab);
  copy(tab.cbegin(),tab.cend(),ostream_iterator<SymCode>(cout,"\n"));

  cout << "-----\n";

  reverseCodes(tab);
  Decoder d(tab);
  d.printTable(cout);

  return 0;
}
