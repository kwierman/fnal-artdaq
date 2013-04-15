#ifndef artdaq_DAQdata_RawEvent_hh
#define artdaq_DAQdata_RawEvent_hh

#include "artdaq/DAQdata/features.hh"
#include "artdaq/DAQdata/Fragments.hh"

#include "cetlib/exception.h"

#include <stddef.h>
#include <iosfwd>
#include <memory>
#include <algorithm>

namespace artdaq {

  // RawEvent is the artdaq view of a generic event, containing a
  // header and zero or more Fragments.

  // RawEventHeader is the artdaq generic event header. It contains
  // the information necessary for routing of raw events inside the
  // artdaq code, but is not intended for use by any experiment.

  namespace detail { struct RawEventHeader; }

  struct detail::RawEventHeader {
    typedef uint32_t                run_id_t;    // Fragments don't know about runs
    typedef uint32_t                subrun_id_t; // Fragments don't know about subruns
    typedef Fragment::sequence_id_t sequence_id_t;

    run_id_t run_id;       // RawDataType run_id;
    subrun_id_t subrun_id; // RawDataType subrun_id;
    sequence_id_t sequence_id;   // RawDataType sequence_id;
    bool is_complete;

    RawEventHeader(run_id_t run,
                   subrun_id_t subrun,
                   sequence_id_t event) :
      run_id(run),
      subrun_id(subrun),
      sequence_id(event),
      is_complete(false)
    { }

  };

  // RawEvent should be a class, not a struct; it should be enforcing
  // invariants (the contained Fragments should all have the correct
  // event id).

  class RawEvent {
  public:
    typedef detail::RawEventHeader::run_id_t    run_id_t;
    typedef detail::RawEventHeader::subrun_id_t subrun_id_t;
    typedef detail::RawEventHeader::sequence_id_t  sequence_id_t;

    RawEvent(run_id_t run, subrun_id_t subrun, sequence_id_t event);

    // Insert the given (pointer to a) Fragment into this
    // RawEvent. This takes ownership of the Fragment referenced by
    // the FragmentPtr, unless an exception is thrown.
#if USE_MODERN_FEATURES
    void insertFragment(FragmentPtr && pfrag);

    // Mark the event as complete
    void markComplete();

    // Return the number of fragments this RawEvent contains.
    size_t numFragments() const;

    // Return the sum of the word counts of all fragments in this RawEvent.
    size_t wordCount() const;

    // Accessors for header information
    run_id_t runID() const;
    subrun_id_t subrunID() const;
    sequence_id_t sequenceID() const;
    bool isComplete() const;

    // Print summary information about this RawEvent to the given stream.
    void print(std::ostream & os) const;

    // Release all the Fragments from this RawEvent, returning them to
    // the caller through a unique_ptr that manages a vector into which
    // the Fragments have been moved.
    std::unique_ptr<std::vector<Fragment>> releaseProduct();

    // Fills in a list of unique fragment types from this event
    void fragmentTypes(std::vector<Fragment::type_t>& type_list);

    // Release the Fragments from this RawEvent with the specified
    // fragment type, returning them to the caller through a unique_ptr
    // that manages a vector into which the Fragments have been moved.
    // PLEASE NOTE that releaseProduct and releaseProduct(type_t) can not
    // both be used on the same RawEvent since each one gives up
    // ownership of the fragments within the event.
    std::unique_ptr<std::vector<Fragment>> releaseProduct(Fragment::type_t);

#endif

  private:
    detail::RawEventHeader header_;
    FragmentPtrs           fragments_;
  };

  typedef std::shared_ptr<RawEvent> RawEvent_ptr;

  inline
  RawEvent::RawEvent(run_id_t run, subrun_id_t subrun, sequence_id_t event) :
    header_(run, subrun, event),
    fragments_()
  { }

#if USE_MODERN_FEATURES
  inline
  void RawEvent::insertFragment(FragmentPtr && pfrag)
  {
    if (pfrag == nullptr) {
      throw cet::exception("LogicError")
          << "Attempt to insert a null FragmentPtr into a RawEvent detected.\n";
    }
    fragments_.emplace_back(std::move(pfrag));
  }

  inline void RawEvent::markComplete() { header_.is_complete = true; }

  inline
  size_t RawEvent::numFragments() const
  {
    return fragments_.size();
  }

  inline
  size_t RawEvent::wordCount() const
  {
    size_t sum = 0;
    for (auto const & frag : fragments_) { sum += frag->size(); }
    return sum;
  }

  inline RawEvent::run_id_t RawEvent::runID() const { return header_.run_id; }
  inline RawEvent::subrun_id_t RawEvent::subrunID() const { return header_.subrun_id; }
  inline RawEvent::sequence_id_t RawEvent::sequenceID() const { return header_.sequence_id; }
  inline bool RawEvent::isComplete() const { return header_.is_complete; }

  inline
  std::unique_ptr<std::vector<Fragment>> RawEvent::releaseProduct()
  {
    std::unique_ptr<std::vector<Fragment>> result(new std::vector<Fragment>);
    result->reserve(fragments_.size());
    for (size_t i = 0, sz = fragments_.size(); i < sz; ++i) {
      result->emplace_back(std::move(*fragments_[i]));
    }
    // It seems more hygenic to clear fragments_ rather than to leave
    // it full of unique_ptrs to Fragments that have been plundered by
    // the move.
    fragments_.clear();
    return result;
  }

  inline
  void RawEvent::fragmentTypes(std::vector<Fragment::type_t>& type_list)
  {
    for (size_t i = 0, sz = fragments_.size(); i < sz; ++i) {
      Fragment::type_t fragType = fragments_[i]->type();
      if (std::find(type_list.begin(),type_list.end(),fragType) == type_list.end()) {
        type_list.push_back(fragType);
      }
    }
    //std::sort(type_list.begin(), type_list.end());
    //std::unique(type_list.begin(), type_list.end());
  }

  inline
  std::unique_ptr<std::vector<Fragment>>
  RawEvent::releaseProduct(Fragment::type_t fragment_type)
  {
    std::unique_ptr<std::vector<Fragment>> result(new std::vector<Fragment>);
    FragmentPtrs::iterator iter = fragments_.begin();
    do {
      if ((*iter)->type() == fragment_type) {
        result->push_back(std::move(*(*iter)));
        iter = fragments_.erase(iter);
      }
      else {
        ++iter;
      }
    } while (iter != fragments_.end());
    return result;
  }

  inline
  std::ostream & operator<<(std::ostream & os, RawEvent const & ev)
  {
    ev.print(os);
    return os;
  }

#endif
}

#endif /* artdaq_DAQdata_RawEvent_hh */
