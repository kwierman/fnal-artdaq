#include "SimpleQueueReader.hh"
#include "DAQdata/RawData.hh"

#include <cassert>
#include <string>
#include <iostream>


void printRawEvent(artdaq::RawEvent const& rawEvent)
{
  std::cout << "Run " << rawEvent.header_.run_id_
            << ", Event " << rawEvent.header_.event_id_
            << ", FragCount " << rawEvent.fragments_.size()
            << ", WordCount " << rawEvent.header_.word_count_
            << std::endl;
  for (int idx = 0; idx < (int) rawEvent.fragments_.size(); ++idx) {
    artdaq::Fragment const& frag = *(rawEvent.fragments_[idx]);
    artdaq::RawFragmentHeader const* rfh = frag.fragmentHeader();
    std::cout << "  Fragment " << rfh->fragment_id_
              << ", WordCount " << rfh->word_count_
              << ", Event " << rfh->event_id_
              << std::endl;
    artdaq::DarkSideHeaderOverlay* dsh =
      (artdaq::DarkSideHeaderOverlay*)(((char*)rfh) + sizeof(artdaq::RawFragmentHeader));
    std::cout << "    DarkSide50: Event Size = " << dsh->event_size_
              << ", Board ID " << dsh->board_id_
              << ", Event Counter " << dsh->event_counter_
              << std::endl;
  }
}

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
        if (doPrint) printRawEvent(*rawEventPtr);
      }
      else {
        usleep(250000);
      }
    }
    if (eventsSeen != expectedEventCount_)
    { throw std::string("Wrong number of events in SimpleQueueReader\n"); }
  }
}
