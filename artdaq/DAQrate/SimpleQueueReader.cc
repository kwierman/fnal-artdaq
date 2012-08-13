#include "artdaq/DAQrate/SimpleQueueReader.hh"

#include <chrono>     // for milliseconds
#include <cstddef>    // for std::size_t
#include <iostream>
#include <string>
#include <thread>     // for sleep_for

namespace artdaq {

  int simpleQueueReaderApp(int, char **)
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
    char * doPrint = getenv("VERBOSE_QUEUE_READING");
    while (true) {
      RawEvent_ptr rawEventPtr;
      if (queue_.deqNowait(rawEventPtr)) {
        // If we got a null pointer, we're done...
        if (!rawEventPtr) { break; }
        ++eventsSeen;
        // Otherwise, do our work ...
        if (doPrint) { std::cout << *rawEventPtr << std::endl; }
      }
      else {
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
      }
    }
    if (expectedEventCount_ && eventsSeen != expectedEventCount_)
    { throw std::string("Wrong number of events in SimpleQueueReader\n"); }
  }
}
