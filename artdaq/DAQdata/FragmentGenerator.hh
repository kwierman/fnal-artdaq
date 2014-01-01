#ifndef artdaq_DAQdata_FragmentGenerator_hh
#define artdaq_DAQdata_FragmentGenerator_hh

////////////////////////////////////////////////////////////////////////
// FragmentGenerator is an abstract class that defines the interface for
// obtaining events in artdaq. Subclasses are to override the (private) virtual
// functions; users of FragmentGenerator are to invoke the public
// (non-virtual) functions.
//
// getNext() will be called only from a single thread
////////////////////////////////////////////////////////////////////////

#include "artdaq/DAQdata/Fragments.hh"

namespace artdaq {
  class FragmentGenerator {
  public:

    FragmentGenerator() = default;

    virtual ~FragmentGenerator() = default;

    // Obtain the next collection of Fragments. Return false to indicate
    // end-of-data. Fragments may or may not be in the same event;
    // Fragments may or may not have the same FragmentID. Fragments
    // will all be part of the same Run and SubRun.
    virtual bool getNext(FragmentPtrs & output) = 0;


    // John F., 12/11/13 -- uncertain what the meaning of the comment below is

    // This generator produces fragments with what distinct IDs (*not*
    // types)?
    virtual std::vector<Fragment::fragment_id_t> fragmentIDs() = 0;

  };

}

#endif /* artdaq_DAQdata_FragmentGenerator_hh */
