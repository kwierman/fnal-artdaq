
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/DS50data.hh"
#include "art/Persistency/Common/Wrapper.h"
#include <vector>

namespace {
  struct dictionary
  {
    artdaq::RawDataType x1;
    artdaq::Fragment    x2;
    std::vector<artdaq::Fragment> x3;
    art::Wrapper<std::vector<artdaq::Fragment> > x4;
  };
}
