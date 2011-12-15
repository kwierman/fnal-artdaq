
#include <iostream>
#include <fstream>
#include <string>
#include "HuffmanTable.hh"

using std::cerr;
using std::ifstream;
using std::string;

int main(int argc, char* argv[])
{
  if (argc < 3) {
    cerr << "Usage: "
         << argv[0]
         << " training_set_file_name table_output_file_name\n";
    return -1;
  }
  ifstream ifs(argv[1], std::ios::binary);
  HuffmanTable h(ifs, Properties::count_max());
  h.writeTable(argv[2]);
  h.writeTableReversed(string(argv[2])+"_reversed.txt");
  return 0;
}


