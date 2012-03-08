#ifndef artdaq_DAQrate_Fragment_hh
#define artdaq_DAQrate_Fragment_hh

typedef long ElementType;

struct FragHeader
{
  ElementType frag_words_;
  ElementType from_;
  ElementType id_;
  ElementType time_ms_; // length of time to process
};

#endif /* artdaq_DAQrate_Fragment_hh */
