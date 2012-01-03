# This makefile is for building the compression library on Mac OS X, where
# we can't use the CET build system yet.
CXX=/opt/local/bin/g++-mp-4.6

# We don't want to use -D_GLIBCXX_PARALLEL until we make sure all our use
# of standard library algorithms are parallel-safe.
CXXFLAGS=-std=c++0x -g -O2 -Wall -Wextra -fopenmp

.PHONY: clean cleandat dat test

all: libHuffman.so MakeSample BuildHuffmanTable EncodeHuffman DecodeHuffman tester Codes

test: samples.dat uncomp.dat
	@(diff samples.dat uncomp.dat && echo TEST OK) || echo TEST FAIL

samples.dat samples.dat2: MakeSample Makefile
	./MakeSample 0 4 33000 samples.dat

samples_training.dat samples_training.dat2: MakeSample
	./MakeSample 0 4 10000000 samples_training.dat

table.txt: BuildHuffmanTable samples_training.dat
	./BuildHuffmanTable samples_training.dat $@

comp.dat: EncodeHuffman table.txt samples.dat
	./EncodeHuffman table.txt samples.dat $@

uncomp.dat: DecodeHuffman table.txt comp.dat
	./DecodeHuffman table.txt comp.dat $@

dat: uncomp.dat

libHuffman.so: SymProb.o SymTable.o Decoder.o Encoder.o HuffmanTable.o BlockReader.o
	$(LINK.cc) -shared $+  -o $@

SymProb.o: SymProb.cc SymProb.hh Properties.hh
SymTable.o: SymTable.cc SymProb.hh Properties.hh
SymCode.o: SymCode.cc SymCode.hh Properties.hh
Decoder.o: Decoder.cc Decoder.hh Properties.hh SymCode.hh
Encoder.o: Encoder.cc Encoder.hh Properties.hh SymCode.hh
HuffmanTable.o: HuffmanTable.cc Tree.hh Properties.hh SymCode.hh SymProb.hh

tester.o: tester.cc Properties.hh HuffmanTable.hh Decoder.hh
MakeSample.o: MakeSample.cc Properties.hh
BuildHuffmanTable.o: BuildHuffmanTable.cc Properties.hh SymProb.hh SymCode.hh Tree.hh
Codes.o: Codes.cc Properties.hh SymCode.hh
EncodeHuffman.o: EncodeHuffman.cc Properties.hh SymCode.hh
DecodeHuffman.o: DecodeHuffman.cc Properties.hh SymCode.hh

MakeSample: MakeSample.o 
	$(LINK.cc) -o $@ $^

BuildHuffmanTable: BuildHuffmanTable.o libHuffman.so
	$(LINK.cc) -o $@ $^

Codes: Codes.o libHuffman.so
	$(LINK.cc) -o $@ $^

EncodeHuffman: EncodeHuffman.o libHuffman.so
	$(LINK.cc) -o $@ $^

DecodeHuffman: DecodeHuffman.o libHuffman.so
	$(LINK.cc) -o $@ $^

tester: tester.o  libHuffman.so
	$(LINK.cc) -o $@ $^

cleandat:
	/bin/rm -f samples_training.dat{,2} samples.dat{,2} table.txt{,_reversed.txt} comp.dat uncomp.dat

clean: cleandat
	/bin/rm -f *.so
	/bin/rm -f *.o *.gch EncodeHuffman DecodeHuffman BuildHuffmanTable
	/bin/rm -f MakeSample tester
