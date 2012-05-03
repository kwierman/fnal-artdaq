#include "artdaq/DAQdata/DS50FragmentSimulator.hh"
#include "artdaq/DAQdata/DS50Board.hh"
#include "artdaq/DAQdata/detail/DS50Header.hh"

#include "fhiclcpp/ParameterSet.h"

using namespace artdaq;

namespace {
  void read_adc_freqs(std::string const & fileName);
}

ds50::FragmentSimulator::FragmentSimulator(fhicl::ParameterSet const & ps) :
  current_event_num_(0),
  events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
  fragments_per_event_(ps.get<size_t>("fragments_per_event", 5)),
  starting_fragment_id_(ps.get<size_t>("starting_fragment_id", 0)),
  nChannels_(ps.get<size_t>("nChannels", 600000)),
  run_number_(ps.get<RawDataType>("run_number")),
  engine_(ps.get<int64_t>("random_seed", 314159)),
  adc_freqs_(),
  content_generator_(fragments_per_event_)
{
  read_adc_freqs(ps.get<std::string>("freqs_file"));
  for (size_t i = 0, id = starting_fragment_id_;
       i < fragments_per_event_;
       ++i, ++id) {
    content_generator_.emplace_back(Board::adc_range(),
                                    -0.5,
                                    Board::adc_range() - 0.5,
                                    [this, id](double x) -> double { return adc_freqs_[id][std::round(x)]; }
                                   );
  }
}

ds50::FragmentSimulator::~FragmentSimulator()
{ }

bool
ds50::FragmentSimulator::getNext_(FragmentPtrs & frags)
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
    auto & newfrag(*frags.back());
    // TODO: Hide numerology.
    newfrag.resize((detail::Header::size_words + nChannels_ / 2) / 2);
    // TODO: Should have a class for this.
    detail::Header h { static_cast<detail::Header::event_size_t>(detail::Header::size_words + nChannels_ / 2),
        0,
        0,
        0,
        0,
        static_cast<detail::Header::board_id_t>(fragID),
        static_cast<detail::Header::event_counter_t>(current_event_num_),
        0,
        0
        };
    memcpy(&*newfrag.dataBegin(),
           &h,
           sizeof(h));
    std::generate_n(newfrag.dataBegin() + detail::Header::size_words / 2,
                    nChannels_,
                    [this,fragID]() {
                      return static_cast<int>
                        (std::round(content_generator_[fragID](engine_)));
                    }
                   );
  }
  return true;
}
