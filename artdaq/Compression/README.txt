
to run this stuff:

$ ./MakeSample 0 4 5000 samples.dat
$ ./BuildHuffmanTable samples.dat table.txt
$ ./EncodeHuffman table.txt samples.dat comp.dat
$ ./DecodeHuffman table.txt comp.dat uncomp.dat

This sequences make a sample of unsigned shorts (the ADC counts),
then it builds a huffman table based on the samples and dumps it
to a text file, then it compresses the samples using the 
huffman codes in the table.

Most of the code is in its second version.  Nothing is validated.
The new version of the DecodeHuffman still has problems and does 
not work.

The DS50Run002211.txt file is Huffman table for 100x10^6 samples
from DS50 run 002211.

