#ifndef simplequeuereader_hhh
#define simplequeuereader_hhh

#include "ConcurrentQueue.hh"
#include "RawData.hh"
#include <thread>

class SimpleQueueReader
{
public:
  SimpleQueueReader(std::shared_ptr<daqrate::ConcurrentQueue< 
                    std::shared_ptr<RawEvent> > > queue);
  ~SimpleQueueReader();

  void requestStop();
  void run();

private:
  std::shared_ptr<daqrate::ConcurrentQueue< std::shared_ptr<RawEvent> > > queue_;

  bool thread_stop_requested_;
  std::thread* reader_thread_;
};

#endif
