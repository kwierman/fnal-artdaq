
#include "EventStore.hh"
#include "Fragment.hh"
#include "Perf.hh"
#include <utility>
#include <cstring>		// memcpy

using namespace std;

namespace artdaq
{

  EventStore::EventStore(Config const& conf):sources_(conf.sources_)
  {
    thread_stop_requested_ = false;
    reader_thread_ = new std::thread(std::bind(&EventStore::run, this));
  }


  EventStore::~EventStore()
  {
    // stop and clean up the reader thread
    thread_stop_requested_ = true;
    reader_thread_->join();
    delete reader_thread_;
  }


  void EventStore::operator()(Data const& ef)
  {
    // find the event being built and put the fragment into it,
    // start new event if not already present
    // if the event is complete, delete it and report timing

    // make a perf record for a new event
    // make a perf record for the completion of an event

    FragHeader* fh = (FragHeader*)&ef[0];
    long event_id = fh->id_;
    pair<EventMap::iterator,bool> p = events_.insert(EventMap::value_type(event_id,0));

    RawEvent::FragmentPtr fp(new Fragment(fh->frag_words_));
  
    std::shared_ptr<RawEvent>   resp(new RawEvent());
    resp->fragment_list_.push_back(fp);

    //queue_.enqNowait( resp );

    if(p.second==true)
      PerfWriteEvent(EventMeas::START,event_id);

    ++((p.first)->second);

    if(p.first->second == sources_)
      {
	PerfWriteEvent(EventMeas::END,event_id);
	events_.erase(p.first);
      }  
  }


  void EventStore::run()
  {
    //for (int idx = 0; idx < 10; ++idx) {
    //  if (thread_stop_requested_) {break;}
    //  sleep(1);
    //  std::cout << "Thread Loop " << idx << std::endl;
    //}
  }
}
