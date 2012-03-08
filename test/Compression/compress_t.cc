#include "artdaq/Compression/Decoder.hh"
#include "artdaq/Compression/Encoder.hh"
#include "artdaq/Compression/Properties.hh"

#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>

ADCCountVec input_data
( {
  484, 311,  342,  497,  373,  466,  547,  538,  421,  384,  552,  505,
  391,  481,  544,  756,  620,  447,  563,  475,  543,  438,  482,
  503,  417,  557,  520,  692,  507,  288,  540,  421
}
);


/*
  These are the expected compression results.
  They must be read from bottom to top, and
  from right to left.

      01 1100 0000 0100 1000 0010 1010 0000
    0010 1000 0101 1010 0000 1111 0000 0000
    0110 0000 0110 1000 0101 0110 0001 1000

    1000 0110 0110 0001 1011 0000 0111 1010
    0001 1010 1000 0101 0010 0000 1101 1000
    0100 1010 0000 0011 0000 0000 0010 0000

    1100 1000 0001 0110 0001 1111 0000 0010
    1100 0000 1110 0000 1110 0100 0010 0110
    0000 1110 1100 0010 0011 0000 0100 1100
    0010 0001 0000 0111 0100 0001 0001 0000
*/

DataVec expected_compression_results
( {
  0x0ec2304c21074110,
  0xc8161f02c0e0e426,
  0x1a8520d84a030020,
  0x606856188661b07a,
  0x1c0482a0285a0f00
}
);

int main()
{
  int retval = 0;
  // Create the Encoder object, starting from the training sample.
  SymTable    symbols;
  readTable("thirty-two-12-bit-table.txt", symbols, Properties::count_max());
  Encoder encoder(symbols);
  std::cerr << "debug: made encoder\n";
  DataVec compression_buffer(chunk_size_counts);
  // NOTE: The size of compression_buffer is large enough so that, for
  // this test, we will not fill it.
  size_t bits_encoded = encoder(input_data, compression_buffer);
  assert(bits_encoded == 318);
  for (size_t i = 0, sz = expected_compression_results.size();
       i != sz; ++i) {
    if (expected_compression_results[i] != compression_buffer[i]) {
      ++retval;
      std::cerr << "Entry " << i << " failed to match"
                << "\n  expected: " << expected_compression_results[i]
                << "\n  observed: " << compression_buffer[i]
                << '\n';
    }
  }
  //----------------------------------------------------------
  // Now we decode.
  Decoder decoder(symbols);
  ADCCountVec decompressed_data;
  decoder(bits_encoded, expected_compression_results, decompressed_data);
  assert(decompressed_data == input_data);
  return retval;
}
