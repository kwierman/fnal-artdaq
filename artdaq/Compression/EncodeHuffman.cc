
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

#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymCode.hh"
#include "artdaq/Compression/BlockReader.hh"
#include "artdaq/Compression/Encoder.hh"

using namespace std;

int main(int argc, char* argv[])
{
  if (argc < 4) {
    cerr << "Usage: " << argv[0] << " huff_table data_file_in data_file_out\n";
    return -1;
  }
  SymTable t;
  readTable(argv[1], t, Properties::count_max());
  ifstream data_ifs(argv[2], std::ios::binary);
  ofstream data_ofs(argv[3], std::ios::binary);
  BlockReader br(data_ifs);
  ADCCountVec in;
  DataVec out(chunk_size_counts);
  Encoder en(t);
  size_t word_count = 0;
  while ((word_count = br.next(in))) {
    reg_type bit_count = en(in, out);
    data_ofs.write((const char*)&bit_count, sizeof(reg_type));
    data_ofs.write((const char*)&out[0], bitCountToBytes(bit_count));
  }
  return 0;
}
