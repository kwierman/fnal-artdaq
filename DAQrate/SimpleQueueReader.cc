#include "SimpleQueueReader.hh"
#include "DAQdata/DS50data.hh"

#include <cassert>
#include <iostream>
#include <string>

namespace artdaq {

  int simpleQueueReaderApp(int, char**)
  {
    try {
      SimpleQueueReader reader;
      reader.run();
      return 0;
    }
    catch (std::string const & msg) {
      std::cerr << "simpleQueueReaderApp failed: "
                << msg;
      return 1;
    }
    catch (...) {
      return 1;
    }
  }

  SimpleQueueReader::
  SimpleQueueReader(std::size_t eec) :
    queue_(getGlobalQueue()),
    expectedEventCount_(eec)
  {  }

  void SimpleQueueReader::run()
  {
    std::size_t eventsSeen = 0;
    char* doPrint = getenv("VERBOSE_QUEUE_READING");
    while (true) {
      RawEvent_ptr rawEventPtr;
      if (queue_.deqNowait(rawEventPtr)) {
        // If we got a null pointer, we're done...
        if (!rawEventPtr) { break; }
        ++eventsSeen;
        // Otherwise, do our work ...
        if (doPrint) std::cout << *rawEventPtr << std::endl;
      }
      else {
        usleep(250000);
      }
    }
    if (eventsSeen != expectedEventCount_)
    { throw std::string("Wrong number of events in SimpleQueueReader\n"); }
  }
}
