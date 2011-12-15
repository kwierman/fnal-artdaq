#include "Compression/HuffmanTable.hh"
#include "Compression/Properties.hh"
#include "Compression/Encoder.hh"

#include <algorithm>
#include <fstream>
#include <iostream>

void printDataVec(std::ostream& os, DataVec const& d)
{
  std::for_each(d.cbegin(), d.cend(),
		[&](DataVec::value_type v)
		{ os << v << ","; }
		);
  os << '\n';
}

DataVec expected_compression_results( {} ); 

int main()
{
  int retval = 0;
  // This is the file we'll use as both the training sample and the
  // data file to be encoded.
  const char* sample_file = "thirty-two-12-bit.dat";

  // Create the HuffmanTable object based on our training sample.
  std::ifstream training_stream(sample_file, std::ios::binary);
  if (!training_stream)
    {
      std::cerr << "Failed to open input file\n";
      return 1;
    }
  std::cerr << "debug: opened training sample\n";
  HuffmanTable  hufftable(training_stream, Properties::count_max());
  std::cerr << "debug: made hufftable\n";
  
  // Create the Encoder object, starting from the training sample.
  SymTable    symbols;
  hufftable.extractTable(symbols);
  std::cerr << "debug: extracted symbols\n";
  Encoder encoder(symbols);
  std::cerr << "debug: made encoder\n";
  
  // Create the BlockReader, which we'll use to read the data we are
  // going to encode.
  std::ifstream input_stream(sample_file, std::ios::binary);
  BlockReader  reader(input_stream);
  std::cerr << "debug: made blockreader\n";
  
  // Loop over the input data; use the BlockReader to read from the
  // input stream, writing to the input_data buffer. The compressed
  // data is written to the output beuffer compressed_data.
  ADCCountVec  input_data;
  DataVec compression_buffer(chunk_size_counts);
  
  // NOTE: The size of compression_buffer is large enough so that, for
  // this test, we will not fill it.
  size_t word_count = 0;
  while ((word_count = reader.next(input_data)))
    {
      encoder(input_data, compression_buffer);
    }

  std::cerr << "debug: done with loop over reader\n";

  if (compression_buffer != expected_compression_results)
    {
      std::cerr << "Compression results not as expected"
		<< "\nexpected: ";
      printDataVec(std::cerr, expected_compression_results);
      std::cerr << "\nobtained: ";
      printDataVec(std::cerr, compression_buffer);
      std::cerr << '\n';
      retval = 1;
    }
  
  return retval;
}
