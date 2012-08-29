#include "ds50daq/DAQ/FragmentSimulator.hh"

#include "artdaq/DAQdata/GeneratorMacros.hh"
#include "cetlib/exception.h"
#include "ds50daq/DAQ/DS50Board.hh"
#include "ds50daq/DAQ/DS50BoardWriter.hh"
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
    frags.emplace_back(new Fragment);
    BoardWriter newboard(*frags.back());
    newboard.resize(nChannels_);
    newboard.setBoardID(fragID);
    newboard.setEventCounter(current_event_num_);
    std::generate_n(newboard.dataBegin(),
                    nChannels_,
                    [this, i]() {
                      return static_cast<adc_type>
                        (std::round(content_generator_[i](engine_)));
                    }
                   );
  }
  return true;
}

DEFINE_ARTDAQ_GENERATOR(ds50::FragmentSimulator)
