#ifndef artdaq_Compression_SymTable_hh
#define artdaq_Compression_SymTable_hh

#include "artdaq/Compression/SymCode.hh"
#include <vector>

typedef std::vector<SymCode> SymTable;

void readTable(const char* fname, SymTable& out, size_t countmax);
void writeTable(const char* fname, SymTable const& in);
void reverseCodes(SymTable&);


#endif /* artdaq_Compression_SymTable_hh */
