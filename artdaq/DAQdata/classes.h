#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/DS50CompressedEvent.hh"
#include "art/Persistency/Common/Wrapper.h"
#include <vector>

template class std::vector<artdaq::Fragment>;
template class art::Wrapper<std::vector<artdaq::Fragment> >;

namespace {
  struct dictionary {
    ds50::CompressedEvent d1;
  };
}

template class art::Wrapper<ds50::CompressedEvent>;
