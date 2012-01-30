#ifndef artdaq_DAQrate_FragmentPool_hh
#define artdaq_DAQrate_FragmentPool_hh

// A FragmentPool holds Fragments, and is used to combine them into
// complete events.

/*
  how is works:
  generate 10 times the data needed for one event (random numbers).
  each time a request for an event comes in, generate a starting position in the
  data, a new event number, and a processing time number.  copy this new event
  into the requestors buffer, place the event number, the rank of this process,
  and the milliseconds of processing time required for it in the first three
  words of the event.
 */

#include "Config.hh"
#include "DAQdata/RawData.hh"

#include <vector>
#include <fstream>

class FragmentPool 
{
public:
  explicit FragmentPool(Config const &);
  
  void operator()(artdaq::Fragment& output);
  
private:
  int seq_;
  unsigned word_count_; // in words, not bytes
  int rank_;
  int data_length_;
  artdaq::Fragment d_;
  int range_;
  std::ifstream ifs_;
  int debugPrintLevel_;
  unsigned wordsReadFromFile_;
  unsigned fileBufferWordOffset_;
};

#endif
