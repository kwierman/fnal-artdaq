#ifndef artdaq_DAQdata_Fragments_hh
#define artdaq_DAQdata_Fragments_hh

#include <memory>
#include <vector>
#include "Fragment.hh"

namespace artdaq {
  typedef std::vector<Fragment>     Fragments;
  typedef std::unique_ptr<Fragment> FragmentPtr;
  typedef std::vector<FragmentPtr>  FragmentPtrs;
}

#endif
