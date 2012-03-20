#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/DS50RawData.hh"
#include "art/Persistency/Common/Wrapper.h"
#include <vector>

namespace {
  struct dictionary
  {
    std::vector<artdaq::Fragment> x3;
    art::Wrapper<std::vector<artdaq::Fragment> > x4;
  };
}
