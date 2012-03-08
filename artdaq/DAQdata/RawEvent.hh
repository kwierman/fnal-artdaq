#ifndef artdaq_DAQdata_RawEvent_hh
#define artdaq_DAQdata_RawEvent_hh

#include "artdaq/DAQdata/features.hh"
#include "artdaq/DAQdata/Fragments.hh"

#include "cetlib/exception.h"

#include <stddef.h>
#include <iosfwd>

namespace artdaq
{

  // RawEvent is the artdaq view of a generic event, containing a
  // header and zero or more Fragments.

  // RawEventHeader is the artdaq generic event header. It contains
  // the information necessary for routing of raw events inside the
  // artdaq code, but is not intended for use by any experiment.

  namespace detail { struct RawEventHeader; }

  struct detail::RawEventHeader
  {
    typedef uint32_t             run_id_t;    // Fragments don't know about runs
    typedef uint32_t             subrun_id_t; // Fragments don't know about subruns
    typedef Fragment::event_id_t event_id_t;
    
    run_id_t run_id;       // RawDataType run_id;
    subrun_id_t subrun_id; // RawDataType subrun_id;
    event_id_t event_id;   // RawDataType event_id;

    RawEventHeader(run_id_t run,
                   subrun_id_t subrun,
                   event_id_t event) :
      run_id(run),
      subrun_id(subrun),
      event_id(event)
    { }

  };

  // RawEvent should be a class, not a struct; it should be enforcing
  // invariants (the contained Fragments should all have the correct
  // event id).

  struct RawEvent
  {
    typedef detail::RawEventHeader::run_id_t    run_id_t;
    typedef detail::RawEventHeader::subrun_id_t subrun_id_t;
    typedef detail::RawEventHeader::event_id_t  event_id_t;

    RawEvent(run_id_t run, subrun_id_t subrun, event_id_t event);

    // Insert the given (pointer to a) Fragment into this
    // RawEvent. This takes ownership of the Fragment referenced by
    // the FragmentPtr, unless an exception is thrown.
#if USE_MODERN_FEATURES
    void insertFragment(FragmentPtr&& pfrag);

    // Return the number of fragments this RawEvent contains.
    size_t numFragments() const;

    // Return the sum of the word counts of all fragments in this RawEvent.
    size_t wordCount() const;

    // Accessors for header information
    run_id_t runID() const;
    subrun_id_t subrunID() const;
    event_id_t eventID() const;

    // Print summary information about this RawEvent to the given stream.
    void print(std::ostream& os) const;
#endif

    detail::RawEventHeader header_;
    FragmentPtrs           fragments_;
  };

  inline
  RawEvent::RawEvent(run_id_t run, subrun_id_t subrun, event_id_t event) :
      header_(run,subrun,event),
      fragments_()
    { }

#if USE_MODERN_FEATURES
  inline
  void RawEvent::insertFragment(FragmentPtr&& pfrag)
  {
    if (pfrag == nullptr)
      {
        throw cet::exception("LogicError")
          << "Attempt to insert a null FragmentPtr into a RawEvent detected.\n";
      }

    if (pfrag->eventID() != header_.event_id)
      {
        throw cet::exception("DataCorruption")
          << "Attempt to insert a Fragment from event " << pfrag->eventID()
          << " into a RawEvent with id " << header_.event_id
          << " detected\n";
      }
  
    fragments_.emplace_back(std::move(pfrag));
  }

  inline
  size_t RawEvent::numFragments() const
  {
    return fragments_.size();
  }

  inline
  size_t RawEvent::wordCount() const
  {
    size_t sum = 0;
    for (auto const& frag : fragments_) sum += frag->size();
    return sum;
  }

  inline RawEvent::run_id_t RawEvent::runID() const { return header_.run_id; }
  inline RawEvent::subrun_id_t RawEvent::subrunID() const { return header_.subrun_id; }
  inline RawEvent::event_id_t RawEvent::eventID() const { return header_.event_id; }

  inline
  std::ostream& operator<<(std::ostream& os, RawEvent const& ev)
  {
    ev.print(os);
    return os;
  }

#endif
}

#endif /* artdaq_DAQdata_RawEvent_hh */
