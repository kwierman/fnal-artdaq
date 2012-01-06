
#include "EventStore.hh"
#include "Perf.hh"
#include <utility>
#include <cstring>

using namespace std;

namespace artdaq
{

  EventStore::EventStore(Config const& conf) :
    sources_(conf.sources_),fragmentIdOffset_(conf.srcStart()),run_(conf.run_)
  {
    queue_.reset(new daqrate::ConcurrentQueue< std::shared_ptr<RawEvent> >());
    reader_.reset(new SimpleQueueReader(queue_));
  }


  void EventStore::operator()(Fragment const& ef)
  {
    // find the event being built and put the fragment into it,
    // start new event if not already present
    // if the event is complete, delete it and report timing

    RawFragmentHeader* fh = (RawFragmentHeader*)&ef[0];
    RawDataType event_id = fh->event_id_;

    // update the fragment ID (up to this point, it has been set to the
    // detector rank or the source rank, but now we just want a simple index)
    fh->fragment_id_ -= fragmentIdOffset_;

    std::shared_ptr<RawEvent> rawEventPtr(new RawEvent());
    pair<EventMap::iterator,bool> p =
      events_.insert(EventMap::value_type(event_id, rawEventPtr));

    bool newElementInMap = p.second;
    rawEventPtr = p.first->second;

    if(newElementInMap)
      {
        PerfWriteEvent(EventMeas::START,event_id);

        rawEventPtr->header_.word_count_ = fh->word_count_ +
          (sizeof(RawEventHeader) / sizeof(RawDataType));
        rawEventPtr->header_.run_id_ = run_;
        rawEventPtr->header_.subrun_id_ = 0;
        rawEventPtr->header_.event_id_ = event_id;
      }
    else
      {
        rawEventPtr->header_.word_count_ += fh->word_count_;
      }

    RawEvent::FragmentPtr fp(new Fragment(fh->word_count_));
    memcpy(&(*fp)[0], &ef[0], (fh->word_count_ * sizeof(RawDataType)));
    rawEventPtr->fragment_list_.push_back(fp);

    if((int) rawEventPtr->fragment_list_.size() == sources_)
      {
        PerfWriteEvent(EventMeas::END,event_id);
        events_.erase(p.first);
        queue_->enqNowait( rawEventPtr );
      }  
  }
}
