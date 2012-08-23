#include "artdaq/DAQdata/Fragment.hh"
#include "art/Persistency/Common/Wrapper.h"
#include <vector>

template class std::vector<artdaq::Fragment>;
template class art::Wrapper<std::vector<artdaq::Fragment> >;
