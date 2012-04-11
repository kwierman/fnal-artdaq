#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/DS50RawData.hh"
#include "art/Persistency/Common/Wrapper.h"
#include <vector>

template class std::vector<artdaq::Fragment>;
template class art::Wrapper<std::vector<artdaq::Fragment> >;

namespace {
  struct dictionary {
    ds50::DS50RawData d1;
  };
}

template class art::Wrapper<ds50::DS50RawData>;
