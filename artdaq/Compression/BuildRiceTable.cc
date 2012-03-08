#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>

#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymProb.hh"

using namespace std;

void readTrainingSet(istream & ifs, ADCCountVec & out, size_t max_samples)
{
  bool forever = max_samples == 0 ? true : false;
  const size_t sz = sizeof(adc_type);
  while (forever || max_samples > 0) {
    adc_type data;
    ifs.read((char*)&data, sz);
    if (ifs.eof()) { break; }
    out.push_back(data);
    --max_samples;
  }
}

int main(int argc, char* argv[])
{
  size_t max_samples = 0;
  if (argc < 2) {
    cerr << "Usage: "
         << argv[0]
         << " training_set_file_name [max_samples]\n";
    return -1;
  }
  if (argc == 3) {
    max_samples = atoi(argv[2]);
    // cerr << "optional samples = " << max_samples << "\n";
  }
  ADCCountVec adcs;
  DataVec run_lengths;
  SymsVec probs;
  size_t countmax = Properties::count_max();
  ifstream ifs(argv[1], std::ios::binary);
  readTrainingSet(ifs, adcs, max_samples);
  calculateProbs(adcs, probs, countmax);
#if 1
  // calculate mode
  size_t mode = probs[0].sym;
  // subtract out mode
  for_each(adcs.begin(), adcs.end(),
  [&](ADCCountVec::value_type & v) { v -= mode; });
#endif
  // calculate median
  unsigned long runs_median;
  unsigned long onebits = calculateRunLengths(adcs, run_lengths, runs_median);
  unsigned long totalbits = adcs.size() * sizeof(adc_type) * 8;
  cout << "total samples  = " << adcs.size() << "\n"
       << "number of ones = " << onebits << "\n"
       << "total number   = " << totalbits << "\n"
       << "runs median    = " << runs_median << "\n";
  double p = 1.0 - ((double)onebits / (double)totalbits);
  double m = ceil(-log2(1.0 + p) / log2(p));
  cout << "p = " << p << "\n"
       << "m = " << m << "\n";
  for_each(run_lengths.cbegin(), run_lengths.cend(),
           [&](DataVec::value_type const & v)
  { cout << v << "\n"; });
  return 0;
}
