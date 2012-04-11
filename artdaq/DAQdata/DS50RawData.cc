
#include "artdaq/DAQdata/DS50RawData.hh"

#include <cstring>

namespace ds50 {
  DS50RawData::DS50RawData(std::vector<artdaq::Fragment> const & init):
    ds50_headers_(init.size()),
    compressed_fragments_(init.size()),
    counts_(init.size())
  {
    size_t index = 0;
    std::for_each(init.begin(),
                  init.end(),
    [&index, this](artdaq::Fragment const & frag) {
      memcpy(&ds50_headers_[index++],
             &*frag.dataBegin(),
             sizeof(HeaderProxy));
    });
  }
}
