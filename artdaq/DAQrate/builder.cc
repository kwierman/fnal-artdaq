#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/Config.hh"
#include "artdaq/DAQrate/Debug.hh"
#include "artdaq/DAQrate/DS50FragmentReader.hh"
#include "artdaq/DAQrate/DS50FragmentSimulator.hh"
#include "artdaq/DAQrate/EventStore.hh"
#include "artdaq/DAQrate/FragmentGenerator.hh"
#include "artdaq/DAQrate/MPIProg.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQrate/RHandles.hh"
#include "artdaq/DAQrate/SHandles.hh"
#include "artdaq/DAQrate/SimpleQueueReader.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "artdaq/DAQrate/quiet_mpi.hh"
#include "cetlib/container_algorithms.h"
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"

#include "boost/program_options.hpp"
namespace  bpo = boost::program_options;

#include <iostream>
#include <memory>
#include <utility>

extern "C" {
#include <sys/time.h>
#include <sys/resource.h>
}

// Class Program is our application object.
class Program : public MPIProg {
public:
  Program(int argc, char * argv[]);
  void go();
  void source();
  void sink();
  void detector();

private:
  enum Color_t : int { DETECTOR, SOURCE, SINK };

  void printHost(const std::string & functionName) const;

  Config conf_;
  bool want_sink_;
  size_t source_buffers_;
  size_t sink_buffers_;
  fhicl::ParameterSet daq_control_ps_;
  MPI_Comm local_group_comm_;
};

artdaq::FragmentGenerator *
make_generator(fhicl::ParameterSet const & ps)
{
  if (ps.get<bool>("generate_data"))
  { return new artdaq::DS50FragmentSimulator(ps); }
  else
  { return new artdaq::DS50FragmentReader(ps); }
}


Program::Program(int argc, char * argv[]):
  MPIProg(argc, argv),
  conf_(rank_, procs_, argc, argv),
  want_sink_(true),
  source_buffers_(0),
  sink_buffers_(0),
  daq_control_ps_(),
  local_group_comm_()
{
  conf_.writeInfo();
  configureDebugStream(conf_.rank_, conf_.run_);
  std::ostringstream descstr;
  descstr << argv[0]
          << " <-c <config-file>>";
  bpo::options_description desc(descstr.str());
  desc.add_options()
  ("config,c", bpo::value<std::string>(), "Configuration file.");
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(conf_.art_argc_, conf_.art_argv_).
               options(desc).allow_unregistered().run(), vm);
    bpo::notify(vm);
  }
  catch (bpo::error const & e) {
    std::cerr << "Exception from command line processing in " << argv[0]
              << ": " << e.what() << "\n";
    throw "cmdline parsing error.";
  }
  if (!vm.count("config")) {
    std::cerr << "Expected \"-- -c <config-file>\" fhicl file specification.\n";
    throw "cmdline parsing error.";
  }
  fhicl::ParameterSet pset;
  cet::filepath_lookup lookup_policy("FHICL_FILE_PATH");
  fhicl::make_ParameterSet(vm["config"].as<std::string>(),
                           lookup_policy, pset);
  daq_control_ps_ = pset.get<fhicl::ParameterSet>("daq");
  daq_control_ps_.get_if_present("wantSink", want_sink_);
  source_buffers_ = daq_control_ps_.get<size_t>("source_buffers", 10);
  sink_buffers_ = daq_control_ps_.get<size_t>("sink_buffers", 10);
  PerfConfigure(conf_, 0); // Don't know how many events.
}

void Program::go()
{
  MPI_Barrier(MPI_COMM_WORLD);
  PerfSetStartTime();
  PerfWriteJobStart();
  MPI_Comm_split(MPI_COMM_WORLD, conf_.type_, 0, &local_group_comm_);
  switch (conf_.type_) {
    case Config::TaskSink:
      sink(); break;
    case Config::TaskSource:
      source(); break;
    case Config::TaskDetector:
      detector(); break;
    default:
      throw "No such node type";
  }
  // MPI_Barrier(MPI_COMM_WORLD);
  PerfWriteJobEnd();
}

void Program::source()
{
  printHost("source");
  // needs to get data from the detectors and send it to the sinks
  artdaq::Fragment frag;
  artdaq::RHandles from_d(source_buffers_,
                          conf_.max_initial_send_words_,
                          1, // Direct.
                          conf_.getSrcFriend());
  artdaq::SHandles to_r(sink_buffers_,
                        conf_.max_initial_send_words_,
                        conf_.sinks_,
                        conf_.sink_start_);
  // TODO: For v2.0, use GMP here and in the detector / sink.
  size_t fragments_processed = 0;
  size_t fragments_expected = 0;
  do {
    from_d.recvFragment(frag);
    if (frag.type() == artdaq::Fragment::type_t::END_OF_DATA) {
      // Debug << "Fragment data: " << *frag.dataBegin() << flusher;
      fragments_expected = *frag.dataBegin();
    }
    else {
      ++fragments_processed;
    }
    if (want_sink_) {
      to_r.sendFragment(std::move(frag));
    }
  }
  while ((!fragments_expected) || fragments_processed < fragments_expected);
  Debug << "source waiting " << conf_.rank_ << flusher;
  if (want_sink_) {
    to_r.waitAll();
  }
  from_d.waitAll();
  Debug << "source done " << conf_.rank_ << flusher;
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::detector()
{
  printHost("detector");
  int detector_rank;
  // Should be zero-based, detectors only.
  MPI_Comm_rank(local_group_comm_, &detector_rank);
  assert(!(detector_rank < 0 ));
  std::ostringstream det_ps_name_loc;
  std::vector<std::string> detectors;

  size_t detectors_size = 0;
  if (!(daq_control_ps_.get_if_present("detectors", detectors) &&
        (detectors_size = detectors.size()))) {
    throw cet::exception("Configuration")
      << "Unable to find required sequence of detector "
      << "parameter set names, \"detectors\".";
  }
  fhicl::ParameterSet det_ps =
    daq_control_ps_.get<fhicl::ParameterSet>
    ((detectors_size > static_cast<size_t>(detector_rank))?
     detectors[detector_rank]:
     detectors[0]);
  std::unique_ptr<artdaq::FragmentGenerator> const gen(make_generator(det_ps));
  artdaq::SHandles h(source_buffers_,
                     conf_.max_initial_send_words_,
                     1, // Direct.
                     conf_.getDestFriend());
  MPI_Barrier(local_group_comm_);
  // MPI_Barrier(MPI_COMM_WORLD);
  // not using the run time method
  // TimedLoop tl(conf_.run_time_);
  size_t fragments_per_source = -1;
  daq_control_ps_.get_if_present("fragments_per_source", fragments_per_source);
  artdaq::FragmentPtrs frags;
  size_t fragments_sent = 0;
  while (gen->getNext(frags) && fragments_sent < fragments_per_source) {
    if (!fragments_sent) {
      // Get the detectors lined up first time before we start the
      // firehoses.
      MPI_Barrier(local_group_comm_);
    }
    for (auto & fragPtr : frags) {
      h.sendFragment(std::move(*fragPtr));
      if (++fragments_sent == fragments_per_source) { break; }
      if ((fragments_sent % 100) == 0) {
        // Don't get too far out of sync.
        MPI_Barrier(local_group_comm_);
      }
    }
    frags.clear();
  }
  artdaq::Fragment eod_frag(artdaq::Fragment::InvalidSequenceID,
                            artdaq::Fragment::InvalidFragmentID,
                            artdaq::Fragment::type_t::END_OF_DATA);
  // TODO: for v2.0, use GMP to manage fragment total where we might be
  // sending lots of fragments for lots of events over weeks and weeks
  // of running.
  eod_frag.resize(1);
  *eod_frag.dataBegin() = fragments_sent;
  // Debug << "EOD data = " << *eod_frag.dataBegin() << flusher;
  h.sendFragment(std::move(eod_frag));
  Debug << "detector waiting " << conf_.rank_ << flusher;
  h.waitAll();
  Debug << "detector done " << conf_.rank_ << flusher;
  MPI_Comm_free(&local_group_comm_);
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::sink()
{
  printHost("sink");
  if (want_sink_) {
    // This scope exists to control the lifetime of 'events'
    int sink_rank;
    MPI_Comm_rank(local_group_comm_, &sink_rank);
    artdaq::EventStore::ARTFUL_FCN *reader =
      (daq_control_ps_.get<bool>("useArt", false))?
      &artapp:
      &artdaq::simpleQueueReaderApp;
    artdaq::EventStore events(conf_.detectors_,
                              conf_.run_,
                              sink_rank,
                              conf_.art_argc_,
                              conf_.art_argv_,
                              reader);
    artdaq::RHandles h(sink_buffers_ * conf_.sources_ / conf_.sinks_,
                       conf_.max_initial_send_words_,
                       conf_.sources_,
                       conf_.source_start_);
    size_t sources_sending = conf_.sources_;
    size_t fragments_expected = 0;
    size_t fragments_received = 0;
    do {
      artdaq::FragmentPtr pfragment(new artdaq::Fragment);
      h.recvFragment(*pfragment);
      if (pfragment->type() == artdaq::Fragment::type_t::END_OF_DATA) {
        --sources_sending;
        // TODO: use GMP to avoid overflow possibility.
        // Debug << "fragments expected: " << fragments_expected;
        fragments_expected += *pfragment->dataBegin();
        // Debug << " -> " << fragments_expected << flusher;
      }
      else {
        ++fragments_received;
        events.insert(std::move(pfragment));
      }
    }
    while (sources_sending || fragments_received < fragments_expected);

    // Now we are done collecting fragments, so we can shut down the
    // receive handles.
    h.waitAll();

    // Make the reader application finish, and capture its return
    // status.
    int rc = events.endOfData();
    Debug << "Sink: reader is done, exit status was: " << rc << flusher;

  } // end of lifetime of 'events'
  Debug << "Sink done " << conf_.rank_ << flusher;
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::printHost(const std::string & functionName) const
{
  char * doPrint = getenv("PRINT_HOST");
  if (doPrint == 0) {return;}
  const int ARRSIZE = 80;
  char hostname[ARRSIZE];
  std::string hostString;
  if (! gethostname(hostname, ARRSIZE)) {
    hostString = hostname;
  }
  else {
    hostString = "unknown";
  }
  Debug << "Running " << functionName
        << " on host " << hostString
        << " with rank " << rank_ << "."
        << flusher;
}

void printUsage()
{
  int myid = 0;
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  std::cout << myid << ":"
            << " user=" << asDouble(usage.ru_utime)
            << " sys=" << asDouble(usage.ru_stime)
            << std::endl;
}

int main(int argc, char * argv[])
{
  int rc = 1;
  try {
    Program p(argc, argv);
    std::cerr << "Started process " << p.rank_ << " of " << p.procs_ << ".\n";
    p.go();
    rc = 0;
  }
  catch (std::string & x) {
    std::cerr << "Exception (type string) caught in driver: "
              << x
              << '\n';
    return 1;
  }
  catch (char const * m) {
    std::cerr << "Exception (type char const*) caught in driver: ";
    if (m)
    { std::cerr << m; }
    else
    { std::cerr << "[the value was a null pointer, so no message is available]"; }
    std::cerr << '\n';
  }
  return rc;
}
