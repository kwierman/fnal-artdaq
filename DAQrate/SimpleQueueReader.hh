#ifndef simplequeuereader_hhh
#define simplequeuereader_hhh

#include "GlobalQueue.hh"
#include <thread>
#include <memory>

namespace artdaq
{
  // simpleQueueReaderApp is a function that can be used in place of
  // artapp(), to read RawEvents from the shared RawEvent queue.
  // Note that it ignores both of its arguments.
  int simpleQueueReaderApp(int, char**);

  // SimpleQueueReader will continue to read RawEvents off the queue
  // until it encounters a null pointer, at which point it stops.
  class SimpleQueueReader
  {
  public:
    explicit SimpleQueueReader(std::size_t eec = 100);
    void run();

  private:
    RawEventQueue&  queue_;
    std::size_t     expectedEventCount_;
  };
}

#endif
