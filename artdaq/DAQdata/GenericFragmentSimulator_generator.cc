#include "artdaq/DAQdata/GenericFragmentSimulator.hh"

#include "artdaq/DAQdata/GeneratorMacros.hh"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <limits>
#include <functional>

artdaq::GenericFragmentSimulator::GenericFragmentSimulator(fhicl::ParameterSet const & ps) :
  content_selection_(static_cast<content_selector_t>
                     (ps.get<size_t>("content_selection", 0))),
  payload_size_spec_(ps.get<size_t>("payload_size", 10240)),
  events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
  fragments_per_event_(ps.get<size_t>("fragments_per_event", 5)),
  starting_fragment_id_(ps.get<size_t>("starting_fragment_id", 0)),
  run_number_(ps.get<RawDataType>("run_number", 1)),
  want_random_payload_size_(ps.get<bool>("want_random_payload_size", false)),
  current_event_num_(0),
  engine_(ps.get<int64_t>("random_seed", 314159)),
  payload_size_generator_(payload_size_spec_),
  fragment_content_generator_()
{ }

bool
artdaq::GenericFragmentSimulator::getNext_(FragmentPtrs & frags)
{
  ++current_event_num_;
  if (events_to_generate_ != 0 &&
      current_event_num_ > events_to_generate_) {
    return false;
  }
  Fragment::fragment_id_t fragID(0);
  frags.reserve(frags.size() + fragments_per_event_);
  for (size_t i = starting_fragment_id_; i < fragments_per_event_; ++i) {
    ++fragID;
    frags.emplace_back();
    bool result =
      getNext(current_event_num_, fragID, frags.back());
    if (!result) { return result; }
  }
  return true;
}

bool
artdaq::GenericFragmentSimulator::
getNext(Fragment::sequence_id_t sequence_id,
        Fragment::fragment_id_t fragment_id,
        FragmentPtr & frag_ptr)
{
  frag_ptr.reset(new Fragment(sequence_id, fragment_id));
  size_t payload_size = generateFragmentSize_();
  frag_ptr->resize(payload_size, 0);
  switch (content_selection_) {
    case content_selector_t::EMPTY:
      break; // values are already correct
    case content_selector_t::FRAG_ID:
      std::fill_n(frag_ptr->dataBegin(), payload_size, fragment_id);
      break;
    case content_selector_t::RANDOM:
      std::generate_n(frag_ptr->dataBegin(),
                      payload_size,
                      [&]() -> long {
                        return
                          fragment_content_generator_(engine_);
                      }
                     );
      break;
    case content_selector_t::DEAD_BEEF:
      std::fill_n(frag_ptr->dataBegin(),
                  payload_size,
                  0xDEADBEEFDEADBEEF);
      break;
    default:
      throw cet::exception("UnknownContentSelection")
          << "Unknown content selection: "
          << static_cast<uint8_t>(content_selection_);
  }
  assert(frag_ptr != nullptr);
  return true;
}

std::size_t
artdaq::GenericFragmentSimulator::
generateFragmentSize_()
{
  return want_random_payload_size_ ?
         payload_size_generator_(engine_) :
         payload_size_spec_;
}

DEFINE_ARTDAQ_GENERATOR(artdaq::GenericFragmentSimulator)