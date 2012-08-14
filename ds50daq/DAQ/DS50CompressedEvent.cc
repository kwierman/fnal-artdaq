
#include "ds50daq/DAQ/DS50CompressedEvent.hh"

#include <cstring>

namespace ds50 {
  CompressedEvent::CompressedEvent(std::vector<artdaq::Fragment> const & init):
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
