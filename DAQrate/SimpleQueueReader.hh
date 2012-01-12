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

  // SimpleQueueReader will continue to read RawEvents off the queue
  // until it encounters a null pointer, at which point it stops.

  class SimpleQueueReader
  {
  public:
    SimpleQueueReader();
    ~SimpleQueueReader();

    void run();

  private:
    RawEventQueue&               queue_;
    std::unique_ptr<std::thread> reader_thread_;
  };
}

#endif
