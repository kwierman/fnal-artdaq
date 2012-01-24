#ifndef artdaq_DAQdata_Fragments_hh
#define artdaq_DAQdata_Fragments_hh

#include <memory>
#include <vector>
#include "Fragment.hh"

namespace artdaq {
  typedef std::shared_ptr<Fragment> FragmentPtr;
  typedef std::vector<FragmentPtr> Fragments;
}

#endif
