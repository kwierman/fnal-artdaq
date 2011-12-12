
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

#include "Table.hh"

using namespace std;

int main(int argc, char* argv[])
{
  if(argc<3)
    {
      cerr << "Usage: " << argv[0] << " training_set_file_name table_output_file_name\n";
      return -1;
    }

  ifstream ifs(argv[1],std::ios::binary);
  HuffmanTable h(ifs);
  h.writeTable(argv[2]);
  return 0;
}


