#†ifndef artdaq_DAQrate_GenericFragmentSimulator_hh
#define artdaq_DAQrate_GenericFragmentSimulator_hh

#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoissonT.h"

#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/FragmentGenerator.hh"
#include "fhiclcpp/fwd.h"

namespace artdaq {
  class GenericFragmentSimulator;
}

// GenericFragmentSimulator creates simulated Generic events, with data
// distributed according to a "histogram" provided in the configuration
// data.
//
// With this implementation, a single call to getNext(frags) will return
// a complete event (event ids are incremented automatically); fragment
// ids are sequential.
// Event size and content are both configurable; see the implementation for
// details.

class artdaq::GenericFragmentSimulator : public artdaq::FragmentGenerator {
public:
  explicit GenericFragmentSimulator(fhicl::ParameterSet const & ps);
  virtual ~GenericFragmentSimulator();

  enum class content_selector_t : uint8_t {
    EMPTY = 0,
    FRAG_ID,
    RANDOM,
    DEAD_BEEF
  };

  // Not part of virtual interface: generate a specific fragment.
  bool getNext(Fragment::event_id_t,
               Fragment::fragment_id_t,
               FragmentPtr & frag_ptr);

private:
  virtual bool getNext_(FragmentPtrs & output);

  std::size_t eventSize_();

  // Configuration
  content_selector_t const content_selection_;
  std::size_t const event_size_spec_; // Posson mean if random size wanted.
  std::size_t const events_to_generate_; // Go forever if this is 0
  std::size_t const fragments_per_event_;
  RawDataType const run_number_;
  bool const want_random_event_size_;

// State
  std::size_t current_event_num_;
  std::unique_ptr<CLHEP::HepRandomEngine> engine_;
  CLHEP::RandPoissonT event_size_generator_;
  CLHEP::RandFlat event_content_generator_;
};

#endif /* artdaq_DAQrate_GenericFragmentSimulator_hh */
