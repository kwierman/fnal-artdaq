#ifndef eventpool_HHH
#define eventpool_HHH

// really this holds pieces of an event (one per source)

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
  typedef std::vector<artdaq::RawDataType> Data;

  FragmentPool(Config const &);
  ~FragmentPool();

  void operator()(Data& output);

private:
  int seq_;
  unsigned word_count_; // in words, not bytes
  int rank_;
  int data_length_;
  Data d_;
  int range_;
  std::ifstream ifs_;
};

#endif
