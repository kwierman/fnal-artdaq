#include <iostream>
#include <vector>
#include <list>
#include <utility>
#include <iterator>
#include <fstream>
#include <memory>
#include <algorithm>
#include <limits>
#include <map>
#include <cmath>

#include "Properties.hh"
#include "SymCode.hh"
#include "Decoder.hh"

using namespace std;

void process_bit(reg_type bit, ADCCountVec& values)
{
}

int main(int argc, char* argv[])
{

  if(argc<4)
    {
      cerr << "Usage: " << argv[0] << " huff_table data_file_in data_file_out\n";
      return -1;
    }

  constexpr auto neg_one = ~(0ul);
  SymTable syms;

  readTable(argv[1],syms);
  sort(syms.begin(),syms.end(),
       [&](SymCode const& a, SymCode const & b) { return a.bit_count_ < b.bit_count_; });

  ifstream data_ifs(argv[2],std::ios::binary);
  Decoder dec(syms);
  ofstream data_ofs(argv[3],std::ios::binary);
  DataVec datavals;
  ADCCountVec adcvals;
  
  while(1)
    {
      reg_type bit_count;

      data_ifs.read((char*)&bit_count,sizeof(reg_type));
      if(data_ifs.eof()) break;
      size_t byte_count = bitCountToBytes(bit_count);
      data_ifs.read((char*)&datavals[0],byte_count);
      if(data_ifs.eof()) break;

      dec(bit_count,datavals,adcvals);
      data_ofs.write((char*)&adcvals[0],adcvals.size()*sizeof(adc_type));
    }

  cout << "completed decompression" << endl;

  return 0;
}
