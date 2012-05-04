#include "cetlib/exception.h"
#include "artdaq/DAQdata/DS50FragmentSimulator.hh"
#include "artdaq/DAQdata/DS50Board.hh"
#include "artdaq/DAQdata/detail/DS50Header.hh"

#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>

using namespace artdaq;

namespace {
  void read_adc_freqs(std::string const & fileName,
                      std::vector<std::vector<size_t>> & freqs) {
    std::ifstream is(fileName);
    if (!is) {
      throw cet::exception("FileOpenError")
        << "Unable to open distribution data file "
        << fileName;
    }
    std::string header;
    std::getline(is, header);
    is.ignore(1);
    std::vector<std::string> split_headers;
    cet::split(header, ' ', std::back_inserter(split_headers));
    size_t nHeaders = split_headers.size();
    freqs.clear();
    freqs.resize(nHeaders -1); // Take account of ADC column.
    for (auto & freq : freqs) {
      freq.resize(ds50::Board::adc_range());
    }
    while (is.peek() != EOF) { // If we get EOF here, we're OK.
      if (!is.good()) {
        throw cet::exception("FileReadError")
          << "Error reading distribution data file "
          << fileName;
      }
      ds50::adc_type channel;
      is >> channel;
      for (auto & freq : freqs) {
        is >> freq[channel];
      }
      is.ignore(1);
    }
    is.close();
  }
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
  content_generator_()
{
  content_generator_.reserve(fragments_per_event_);
  read_adc_freqs(ps.get<std::string>("freqs_file"), adc_freqs_);
  for (size_t i = 0;
       i < fragments_per_event_;
       ++i) {
    content_generator_.emplace_back(Board::adc_range(),
                                    -0.5,
                                    Board::adc_range() - 0.5,
                                    [this, i](double x) -> double { return adc_freqs_[i][std::round(x)]; }
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
  detail::Header::board_id_t fragID(starting_fragment_id_);
// #pragma omp parallel for shared(fragID, frags)
// TODO: Allow parallel operation by having multiple engines (with different seeds, of course).
  for (size_t i = 0; i < fragments_per_event_; ++i, ++fragID) {
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
        fragID,
        static_cast<detail::Header::event_counter_t>(current_event_num_),
        0,
        0
        };
    memcpy(&*newfrag.dataBegin(),
           &h,
           sizeof(h));
    std::generate_n(reinterpret_cast<adc_type*>(&*newfrag.dataBegin() + detail::Header::size_words / 2),
                    nChannels_,
                    [this, i]() {
                      return static_cast<adc_type>
                        (std::round(content_generator_[i](engine_)));
                    }
                   );
  }
  return true;
}
