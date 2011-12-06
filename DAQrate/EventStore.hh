#ifndef eventstore_hhh
#define eventstore_hhh

#include "Config.hh"
#include "EventPool.hh"

#include <map>

// bad to get definition for Data from EventPool!

class EventStore
{
public:
  typedef FragmentPool::Data Data;
  typedef std::map<ElementType,int> EventMap;

  EventStore(Config const&);

  void operator()(Data const&);
private:
  int sources_;
  EventMap events_;
};

#endif
