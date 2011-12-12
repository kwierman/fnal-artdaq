
CXX=g++-mp-4.6
CXXFLAGS=-std=c++0x -g -O2 -Wall -Wextra
.PHONY: clean cleandat dat

all: MakeSample BuildHuffmanTable EncodeHuffman DecodeHuffman

samples.dat samples.dat2: MakeSample
	./MakeSample 0 4 5000 samples.dat

table.txt: BuildHuffmanTable samples.dat
	./BuildHuffmanTable samples.dat $@

comp.dat: EncodeHuffman table.txt samples.dat
	./EncodeHuffman table.txt samples.dat $@

uncomp.dat: DecodeHuffman table.txt comp.dat
	./DecodeHuffman table.txt comp.dat $@

dat: uncomp.dat

SymProb.o: SymProb.cc SymProb.hh Properties.hh
	$(COMPILE.cc) $<

SymCode.o: SymCode.cc SymCode.hh Properties.hh
	$(COMPILE.cc) $<

Decoder.o: Decoder.cc Decoder.hh Properties.hh SymCode.hh
	$(COMPILE.cc) $<

Encoder.o: Encoder.cc Encoder.hh Properties.hh SymCode.hh
	$(COMPILE.cc) $<

Table.o: Table.cc Tree.hh Properties.hh SymCode.hh SymProb.hh
	$(COMPILE.cc) $<

MakeSample.o: MakeSample.cc Properties.hh
	$(COMPILE.cc) $<

BuildHuffmanTable.o: BuildHuffmanTable.cc Properties.hh SymProb.hh SymCode.hh Tree.hh
	$(COMPILE.cc) $<

EncodeHuffman.o: EncodeHuffman.cc Properties.hh SymCode.hh
	$(COMPILE.cc) $<

DecodeHuffman.o: DecodeHuffman.cc Properties.hh SymCode.hh
	$(COMPILE.cc) $<

MakeSample: MakeSample.o 
	$(LINK.cc) -o $@ $^

BuildHuffmanTable: BuildHuffmanTable.o SymProb.o SymCode.o Table.o
	$(LINK.cc) -o $@ $^

EncodeHuffman: EncodeHuffman.o Encoder.o SymCode.o
	$(LINK.cc) -o $@ $^

DecodeHuffman: DecodeHuffman.o Decoder.o SymCode.o
	$(LINK.cc) -o $@ $^

cleandat:
	/bin/rm -f samples.dat samples.dat2

clean: cleandat
	/bin/rm -f *.o *.gch EncodeHuffman DecodeHuffman BuildHuffmanTable MakeSample
