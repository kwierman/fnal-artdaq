#ifndef simplequeuereader_hhh
#define simplequeuereader_hhh

#include "GlobalQueue.hh"
#include <thread>
#include <memory>

namespace artdaq
{
  // SimpleQueueReader can be used to test the functioning of the
  // communication of RawEvents through the global event queue.
  // Instances of SimpleQueueReader are not copyable, because we do
  // not want two different readers from the same global queue in one
  // program.


  // Question: why do we have thread_stop_requested? Does the
  // reader_thread_'s function ever access this value?

  class SimpleQueueReader
  {
  public:
    SimpleQueueReader();
    ~SimpleQueueReader();

    void requestStop();
    void run();

  private:
    RawEventQueue&               queue_;
    volatile  bool               thread_stop_requested_;
    std::unique_ptr<std::thread> reader_thread_;
  };
}

#endif
