#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>

#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymTable.hh"

using namespace std;
using namespace ds50;

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

typedef unsigned long code_type;

struct Code {
  code_type value_ : 32;
  code_type length_ : 32;
};

// -----------------------------

code_type unary_code(long n)
{
  long b = 0x8000000000000000L;
  long a = b >> (n - 1);
  unsigned long c = (unsigned long)a >> (sizeof(long) * 8 - (n + 1));
  c &= 0xffffffffffffffeUL;
  // cout << "unary: " << n << " c=" << (void*)c << "\n";
  return c;
}

code_type b_mask(long b)
{
  unsigned long x = (unsigned long)(-1L);
  unsigned long shift = sizeof(long) * 8 - b;
  unsigned long c = x >> (shift - 1);
  c >>= 1;
  // cout << "mask: " << b << " c=" << (void*)c << " s " << shift << "\n";
  return c;
}

template <code_type k>
Code subexp(code_type n)
{
  constexpr code_type a = 1 << k;
  code_type b = n < a ? k : floor(log2(n));
  code_type u = n < a ? 0 : b - k + 1;
  code_type left = unary_code(u) << b; // coded in u+1 bits
  code_type right = b_mask(b) & n;
  Code result;
  // number of bits in result code = u+1+b
  result.value_ = left | right;
  result.length_ = u + 1 + b;
  return result;
}

// ------------------------------

Code pod(code_type n)
{
  Code result;
  if (n == 0) { result.value_ = 0; result.length_ = 1; return result; } // one bit
  unsigned long num_bits = floor(log2(n) + 1);
  result.value_ = n;
  result.length_ = num_bits * 2;
  return result; // length = num_bits*2;
}

// ------------------------------

int main(int argc, char* argv[])
{
  if (argc < 2) {
    cerr << "Usage: "
         << argv[0]
         << " max_codes\n";
    return -1;
  }
  size_t max_codes = atoi(argv[1]);
  SymTable pod_table, subexp0_table, subexp1_table;
  for (size_t i = 0; i < max_codes; ++i) {
    Code p = pod(i);
    Code s0 = subexp<0>(i);
    Code s1 = subexp<1>(i);
    pod_table.push_back(SymCode(i, p.value_, p.length_));
    subexp0_table.push_back(SymCode(i, s0.value_, s0.length_));
    subexp1_table.push_back(SymCode(i, s1.value_, s1.length_));
  }
  ::writeTable("pod_table.txt", pod_table);
  ::writeTable("subexp0_table.txt", subexp0_table);
  ::writeTable("subexp1_table.txt", subexp1_table);
  return 0;
}
