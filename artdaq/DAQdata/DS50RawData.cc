
#include "artdaq/DAQdata/DS50RawData.hh"

namespace ds50
{
  DS50RawData::DS50RawData(std::vector<artdaq::Fragment> const& init):
    ds50_headers_(init.size()),
    compressed_fragments_(init_size()),
    counts_(init.size()),
    frag_headers_(init.size()),
  {
    for(size_t i = 0; i< init.size(); ++i)
      {
	// we have no access to the fragment header location,
	// do we really need it? perhaps not.
	// frag_headers_[i] = ??
	ds50_headers_[i] = *((detail::Header*)&(init[i][0]));
      }
  }
}
