#include "artdaq/DAQrate/GenericFragmentSimulator.hh"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <limits>
#include <functional>

artdaq::GenericFragmentSimulator::GenericFragmentSimulator(fhicl::ParameterSet const & ps) :
  content_selection_(ps.get<content_selector_t>("content_selection",
                                                content_selector_t::EMPTY)),
  event_size_spec_(ps.get<size_t>("event_size", 10240)),
  events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
  fragments_per_event_(ps.get<size_t>("fragments_per_event", 5)),
  run_number_(ps.get<RawDataType>("run_number", 1)),
  want_random_event_size_(ps.get<bool>("want_random_event_size", true)),
  current_event_num_(0),
  engine_(new CLHEP::HepJamesRandom(ps.get<int64_t>("random_seed", 314159))),
  event_size_generator_(*engine_, event_size_spec_),
  event_content_generator_(*engine_)
{ }

artdaq::GenericFragmentSimulator::~GenericFragmentSimulator()
{ }

bool
artdaq::GenericFragmentSimulator::getNext_(FragmentPtrs & frags)
{
  ++current_event_num_;
  if (events_to_generate_ != 0 &&
      current_event_num_ > events_to_generate_) {
    return false;
  }
  RawDataType fragID(0);
  for (size_t i = 0; i < fragments_per_event_; ++i) {
    ++fragID;
    frags.emplace_back(new Fragment(current_event_num_, fragID));
    size_t event_size = eventSize_();
    frags.back()->reserve(event_size); // Allocate correct size now.
    switch (content_selection_) {
    case content_selector_t::EMPTY:
      break;
    case content_selector_t::FRAG_ID:
      std::fill_n(frags.back()->dataBegin(), eventSize_(), fragID);
      break;
    case content_selector_t::RANDOM:
      std::generate_n(frags.back()->dataBegin(),
                      event_size,
                      [&]() -> long {
                        return
                          event_content_generator_.
                          fireInt(std::numeric_limits<long>::max());
                      }
                     );
      break;
    case content_selector_t::DEAD_BEEF:
      std::fill_n(frags.back()->dataBegin(),
                  event_size,
                  0xDEADBEEFDEADBEEF);
      break;
    default:
      throw cet::exception("UnknownContentSelection")
        << "Unknown content selection: "
        << static_cast<uint8_t>(content_selection_);
    }
  }
  return true;
}

std::size_t
artdaq::GenericFragmentSimulator::
eventSize_()
{
  return want_random_event_size_?
    event_size_generator_.fire():
    event_size_spec_;
}
