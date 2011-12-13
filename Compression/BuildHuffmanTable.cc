
#include <iostream>
#include <fstream>
#include "HuffmanTable.hh"

using std::cerr;
using std::ifstream;

int main(int argc, char* argv[])
{
  if (argc < 3) {
    cerr << "Usage: "
         << argv[0]
         << " training_set_file_name table_output_file_name\n";
    return -1;
  }
  ifstream ifs(argv[1], std::ios::binary);
  HuffmanTable h(ifs);
  h.writeTable(argv[2]);
  return 0;
}


