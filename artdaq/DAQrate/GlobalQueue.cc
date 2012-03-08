#include "artdaq/DAQrate/GlobalQueue.hh"

namespace artdaq {

  // In C++03, one would need to use boost::once_flag and
  // boost::call_once to make sure that there is no race condition
  // between threads for the creation of 'theQueue'.
  // in C++11, this is thread-safe. See C++11 6.7p4.

  RawEventQueue& getGlobalQueue() {
    static RawEventQueue theQueue;
    return theQueue;
  }

}
