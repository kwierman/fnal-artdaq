#include "artdaq/DAQdata/Fragment.hh"
#include "art/Persistency/Common/Wrapper.h"
#include "artdaq/DAQdata/PackageBuildInfo.hh"
#include <vector>

template class std::vector<artdaq::Fragment>;
template class art::Wrapper<std::vector<artdaq::Fragment> >;
template class art::Wrapper<artdaq::PackageBuildInfo>;
