
#include "SimpleQueueReader.hh"
#include "DAQdata/RawData.hh"

namespace artdaq
{

  SimpleQueueReader::
  SimpleQueueReader() :
    queue_(getGlobalQueue()),
    thread_stop_requested_(false),
    reader_thread_(new std::thread(std::bind(&SimpleQueueReader::run, this)))
  {
  }


  SimpleQueueReader::~SimpleQueueReader()
  {
    // Request we stop, and wait for the reader_thread_ to finish.
    requestStop();
    reader_thread_->join();
  }


  void SimpleQueueReader::requestStop()
  {
    thread_stop_requested_ = true;
  }


  void SimpleQueueReader::run()
  {
    char* doPrint = getenv("VERBOSE_QUEUE_READING");
    while (! thread_stop_requested_) {
      std::shared_ptr<RawEvent> rawEventPtr;
      if (queue_.deqNowait(rawEventPtr)) {
        if (doPrint != 0) {
          std::cout << "Run " << rawEventPtr->header_.run_id_
                    << ", Event " << rawEventPtr->header_.event_id_
                    << ", FragCount " << rawEventPtr->fragment_list_.size()
                    << ", WordCount " << rawEventPtr->header_.word_count_
                    << std::endl;
          for (int idx=0; idx<(int) rawEventPtr->fragment_list_.size(); ++idx) {
            RawEvent::FragmentPtr rfp = rawEventPtr->fragment_list_[idx];
            RawFragmentHeader* rfh = (RawFragmentHeader*)&(*rfp)[0];
            std::cout << "  Fragment " << rfh->fragment_id_
                      << ", WordCount " << rfh->word_count_
                      << ", Event " << rfh->event_id_
                      << std::endl;
            DarkSideHeaderOverlay* dsh =
              (DarkSideHeaderOverlay*)(((char*)rfh)+sizeof(RawFragmentHeader));
            std::cout << "    DarkSide50: Event Size = " << dsh->event_size_
                      << ", Board ID " << dsh->board_id_
                      << ", Event Counter " << dsh->event_counter_
                      << std::endl;
          }
        }
      }
      else {
        usleep(250000);
      }
    }
  }
}
