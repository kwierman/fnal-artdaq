
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <iterator>
#include <iomanip>
#include <limits>
#include <fstream>
#include <time.h>

#include "Properties.hh"

using namespace std;

/*
  []  Capture nothing (or, a scorched earth strategy?)
  [&] Capture any referenced variable by reference
  [=] Capture any referenced variable by making a copy
  [=, &foo] Capture any referenced variable by making a copy, but capture variable foo by reference
  [bar] Capture bar by making a copy; don't copy anything else
  [this]  Capture the this pointer of the enclosing class

  Examples:
  [] { cout << "Hello.\n"; }();
  [] () { return 1; };
  [] () -> int { return 1; };
  auto func = [] () { cout << "Hello world\n"; };
  func(); // now call the function

  Example of uniform: uniform_real_distribution<> urd;
 */


int main(int argc, char* argv[])
{
  if (argc < 4) {
    cerr << "usage: " << argv[0] << " noise_mean noise_sd total_samples out_file\n\n";
    cerr << " typical mean = 0\n";
    cerr << " typical sd range is [1,7]\n";
    return 0;
  }
  double mean = atof(argv[1]);
  double sd = atof(argv[2]);
  unsigned long total_samples = strtoul(argv[3], 0, 10);
  const char* fname = argv[4];
  string fname2 = string(fname) + "2";
  //time_t seed;
  //time(&seed);
  unsigned long seed = 1234567;
  mt19937 eng(seed);
  normal_distribution<> nd(mean, sd);
  SignalVec sig_samples;
  generate_n(back_inserter(sig_samples), total_samples,
  [&]() {return nd(eng);});
  ADCCountVec adc_samples;
  adc_samples.reserve(sig_samples.size());
  transform(sig_samples.begin(), sig_samples.end(), back_inserter(adc_samples),
  [&](double d) { return Properties::signalToADC(d); });
  ofstream ost(fname, std::ios::binary);
  ofstream ost2(fname2);
  for_each(adc_samples.cbegin(),
           adc_samples.cend(),
           [&](ADCCountVec::value_type s)
  { ost.write((char*)&s, sizeof(s)); ost2 << s << '\n';}
          );
  return 0;
}
