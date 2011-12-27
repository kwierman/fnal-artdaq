
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
#include <list>
#include <sys/time.h>
#include <omp.h>

#include "DAQdata/Fragment.hh"
#include "Properties.hh"
#include "SymCode.hh"
#include "Encoder.hh"

using namespace std;

typedef std::vector<ADCCountVec> CountList;
typedef std::vector<artdaq::DarkSideHeaderOverlay> HeadList;
typedef std::vector<DataVec> DataList;

double getTime()
{
  struct timeval tv;
  gettimeofday(&tv,0);
  return (double)tv.tv_sec + (double)tv.tv_usec/1e6;
}

int main(int argc, char* argv[])
{
  if (argc < 4) {
    cerr << "Usage: " << argv[0] << " huff_table data_file_in data_file_out [num_events]\n";
    return -1;
  }
  SymTable t;
  readTable(argv[1], t, Properties::count_max());

  ifstream data_ifs(argv[2], std::ios::binary);
  ofstream data_ofs(argv[3], std::ios::binary);
  size_t total = argc==5 ? atoi(argv[4]) : -1ul;

  cout << " num_threads = " << omp_get_num_threads()
       << " max_threads = " << omp_get_max_threads()
       << " env = " << getenv("OMP_NUM_THREADS")
       << endl;

  Encoder en(t);
  size_t tot=0;

  ADCCountVec in;
  artdaq::DarkSideHeaderOverlay header;

  CountList counts;
  HeadList heads;

  for(;tot<total;++tot)
    {
      data_ifs.read((char*)&header,sizeof(header));
      if(data_ifs.eof()) break;

      // event_size is number of 32-bit words, including header
      size_t num_bytes = (header.event_size_ * 4) - (4*4);
      in.resize(num_bytes / 2);

      data_ifs.read((char*)&in[0],num_bytes);
      if(data_ifs.eof()) throw "bad read";

      heads.push_back(header);
      counts.push_back(ADCCountVec());
      counts.back().swap(in);
    }

  // for each header/data chunk, compress it and time the compression
  // print out each of the time/chunk to do compression

  DataList compdata(tot);
  DataVec bits(tot);
  vector<double> times(tot);

  double section_start = getTime();

#pragma omp parallel
  {
    size_t i;
#pragma omp for
    for(i=0;i<tot;++i)
      {
#pragma omp task shared(compdata,bits,times)
	{
	  //cout << "T" << omp_get_thread_num() << "E" << endl;
	  DataVec out(heads[i].event_size_);
	  double start_time = getTime();
	  reg_type bit_count = en(counts[i],out);
	  compdata[i].swap(out);
	  bits[i] = (bit_count);
	  times[i] = (getTime() - start_time);
	}
      }
#pragma omp taskwait
  }

  cerr << "section time = " << (getTime() - section_start) << "\n";

  for(size_t i=0;i<tot;++i)
    {
      double bytes = (double)bitCountToBytes(bits[i]);
      double original_bytes = (double)(heads[i].event_size_ * 4);

      cout << i << " " 
	   << original_bytes << " "
	   << times[i] << " " 
	   << (1.0/times[i]) << " "
	   << (bytes/times[i]) << " "
	   << (bytes/original_bytes) << " "
	   << "\n";

      //data_ofs.write((const char*)&heads[i], sizeof(header));
      //data_ofs.write((const char*)&bits[i],sizeof(reg_type));
      //data_ofs.write((const char*)&compdata[i][0], bytes);      
    }

  return 0;
}
