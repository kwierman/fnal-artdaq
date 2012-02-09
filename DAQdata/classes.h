
#include "DAQdata/Fragment.hh"
#include "DAQdata/DS50data.hh"
#include "art/Persistency/Common/Wrapper.h"

namespace {
  struct dictionary
  {
    artdaq::Fragment vomit2;
    artdaq::RawDataType vomit3;
    artdaq::CompressedFragPart vomit4;
    artdaq::CompressedFragParts vomit5;
    artdaq::DarkSideHeader vomit6;
    artdaq::CompressedBoard vomit7;
    art::Wrapper<artdaq::Fragment> vomit9;
  };
}
