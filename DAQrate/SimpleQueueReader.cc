
#include "SimpleQueueReader.hh"
#include <boost/thread/xtime.hpp>

namespace artdaq
{

  SimpleQueueReader::
  SimpleQueueReader(std::shared_ptr<daqrate::ConcurrentQueue< 
                    std::shared_ptr<RawEvent> > > queue) : queue_(queue)
  {
    thread_stop_requested_ = false;
    reader_thread_ = new std::thread(std::bind(&SimpleQueueReader::run, this));
  }


  SimpleQueueReader::~SimpleQueueReader()
  {
    // stop and clean up the reader thread
    requestStop();
    reader_thread_->join();
    delete reader_thread_;
  }


  void SimpleQueueReader::requestStop()
  {
    thread_stop_requested_ = true;
  }


  void SimpleQueueReader::run()
  {
    while (! thread_stop_requested_) {
      std::shared_ptr<RawEvent> rawEventPtr;
      if (queue_->deqNowait(rawEventPtr)) {
        std::cout << "Run " << rawEventPtr->header_.run_id_
                  << ", Event " << rawEventPtr->header_.event_id_
                  << ", FragCount " << rawEventPtr->fragment_list_.size()
                  << ", WordCount " << rawEventPtr->header_.word_count_
                  << std::endl;
        for (int idx=0; idx<(int) rawEventPtr->fragment_list_.size(); ++idx) {
          RawEvent::FragmentPtr rfp = rawEventPtr->fragment_list_[idx];
          RawFragmentHeader* fh = (RawFragmentHeader*)&(*rfp)[0];
          std::cout << "  Fragment " << fh->fragment_id_
                    << ", WordCount " << fh->word_count_
                    << std::endl;
        }
      }
      else {
        usleep(250000);
      }
    }

    //boost::xtime readTime;
    //readTime.sec = 0;
    //readTime.nsec = 250000000;
    //while (! thread_stop_requested_) {
    //  std::shared_ptr<RawEvent> rawEventPtr;
    //  if (queue_->deqTimedWait(rawEventPtr, readTime)) {
    //    std::cout << "Event " << rawEventPtr->header_.event_id_ << std::endl;
    //  }
    //}
  }
}
