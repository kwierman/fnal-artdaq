#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQdata/Debug.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/makeFragmentGenerator.hh"
#include "artdaq/DAQrate/Config.hh"
#include "artdaq/DAQrate/EventStore.hh"
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

#include <algorithm>
#include <cmath>
#include <cstdlib>
extern "C" {
#include <unistd.h>
}
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

  fhicl::ParameterSet getPset(char const * progname);
  void printHost(const std::string & functionName) const;

  Config conf_;
  fhicl::ParameterSet const daq_pset_;
  bool const want_sink_;
  bool const want_periodic_sync_;
  size_t const max_payload_size_words_;
  size_t const source_buffers_;
  size_t const sink_buffers_;
  MPI_Comm local_group_comm_;
};

class FragCounter {
public:
  explicit FragCounter(size_t nSlots);
  void operator()(size_t slot);
  void operator()(size_t slot, size_t inc);

  size_t total() const;
  size_t total(size_t slot) const;

private:
  std::vector<size_t> receipts_;
};

inline
FragCounter::FragCounter(size_t nSlots)
  :
  receipts_(nSlots, 0)
{
}

inline
void
FragCounter::
operator()(size_t slot)
{
  ++receipts_[slot];
}

inline
void
FragCounter::
operator()(size_t slot, size_t inc)
{
  receipts_[slot] += inc;
}

inline
size_t
FragCounter::
total() const
{
  return
    std::accumulate(receipts_.begin(),
                    receipts_.end(),
                    0);
}

inline
size_t
FragCounter::
total(size_t slot) const
{
  return receipts_[slot];
}

Program::Program(int argc, char * argv[]):
  MPIProg(argc, argv),
  conf_(rank_, procs_, argc, argv),
  daq_pset_(getPset(argv[0])),
  want_sink_(daq_pset_.get<bool>("want_sink", true)),
  want_periodic_sync_(daq_pset_.get<bool>("want_periodic_sync", false)),
  max_payload_size_words_(daq_pset_.get<size_t>("max_payload_size_words", 512 * 1024)),
  source_buffers_(daq_pset_.get<size_t>("source_buffers", 10)),
  sink_buffers_(daq_pset_.get<size_t>("sink_buffers", 10)),
  local_group_comm_()
{
  conf_.writeInfo();
  configureDebugStream(conf_.rank_, conf_.run_);
  PerfConfigure(conf_.rank_,
                conf_.run_,
                conf_.type_);
}

void Program::go()
{
  MPI_Barrier(MPI_COMM_WORLD);
  PerfSetStartTime();
  PerfWriteJobStart();
  MPI_Comm_split(MPI_COMM_WORLD, conf_.type_, 0, &local_group_comm_);
  switch (conf_.type_) {
  case Config::TaskSink:
    if (want_sink_) {
      sink();
    }
    else {
      std::string
        msg("WARNING: a sink was instantiated despite want_sink being false:\n"
            "set nsinks to 0 in invocation of daqrate?\n");
      std::cerr << msg;
      MPI_Barrier(MPI_COMM_WORLD);
    }
    break;
  case Config::TaskSource:
    source(); break;
  case Config::TaskDetector:
    detector(); break;
  default:
    throw "No such node type";
  }
  PerfWriteJobEnd();
}

void Program::source()
{
  printHost("source");
  // needs to get data from the detectors and send it to the sinks
  artdaq::Fragment frag;
  artdaq::RHandles from_d(source_buffers_,
                          max_payload_size_words_,
                          1, // Direct.
                          conf_.getSrcFriend());
  artdaq::SHandles to_r(sink_buffers_,
                        max_payload_size_words_,
                        conf_.sinks_,
                        conf_.sink_start_);
  // TODO: For v2.0, use GMP here and in the detector / sink.
  FragCounter countIn(1), countOut(conf_.sinks_);
  size_t fragments_expected = 0;
  do {
    from_d.recvFragment(frag);
    if (frag.type() == artdaq::Fragment::type_t::END_OF_DATA) {
      fragments_expected = *frag.dataBegin();
    }
    else {
      countIn(0);
      if (want_sink_) {
        countOut(to_r.sendFragment(std::move(frag)) - conf_.sink_start_);
      }
    }
  }
  while ((!fragments_expected) ||
         (Debug << "Waiting for "
          << fragments_expected - countIn.total()
          << " fragments." << flusher,
          countIn.total() < fragments_expected));
  Debug << "source waiting " << conf_.rank_ << flusher;
  if (want_sink_) {
    // Send correct EOD fragments to all sinks.
    for (size_t slot = 0;
         slot < static_cast<size_t>(conf_.sinks_);
         ++slot) {
      to_r.sendEODFrag(slot + conf_.sink_start_,
                       countOut.total(slot));
    }
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
  assert(!(detector_rank < 0));
  std::ostringstream det_ps_name_loc;
  std::vector<std::string> detectors;
  size_t detectors_size = 0;
  if (!(daq_pset_.get_if_present("detectors", detectors) &&
        (detectors_size = detectors.size()))) {
    throw cet::exception("Configuration")
        << "Unable to find required sequence of detector "
        << "parameter set names, \"detectors\".";
  }
  fhicl::ParameterSet det_ps =
    daq_pset_.get<fhicl::ParameterSet>
    ((detectors_size > static_cast<size_t>(detector_rank)) ?
     detectors[detector_rank] :
     detectors[0]);
  std::unique_ptr<artdaq::FragmentGenerator> const
    gen(artdaq::makeFragmentGenerator
        (det_ps.get<std::string>("generator"),
         det_ps));
  artdaq::SHandles h(source_buffers_,
                     max_payload_size_words_,
                     1, // Direct.
                     conf_.getDestFriend());
  MPI_Barrier(local_group_comm_);
  // not using the run time method
  // TimedLoop tl(conf_.run_time_);
  size_t fragments_per_source = -1;
  daq_pset_.get_if_present("fragments_per_source", fragments_per_source);
  artdaq::FragmentPtrs frags;
  size_t fragments_sent = 0;
  while (fragments_sent < fragments_per_source && gen->getNext(frags)) {
    if (!fragments_sent) {
      // Get the detectors lined up first time before we start the
      // firehoses.
      MPI_Barrier(local_group_comm_);
    }
  for (auto & fragPtr : frags) {
      h.sendFragment(std::move(*fragPtr));
      if (++fragments_sent == fragments_per_source) { break; }
      if (want_periodic_sync_ && (fragments_sent % 100) == 0) {
        // Don't get too far out of sync.
        MPI_Barrier(local_group_comm_);
      }
    }
    frags.clear();
  }
  h.sendEODFrag(conf_.getDestFriend(), fragments_sent);
  Debug << "detector waiting " << conf_.rank_ << flusher;
  h.waitAll();
  Debug << "detector done " << conf_.rank_ << flusher;
  MPI_Comm_free(&local_group_comm_);
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::sink()
{
  printHost("sink");
  {
    // This scope exists to control the lifetime of 'events'
    int sink_rank;
    bool useArt = daq_pset_.get<bool>("useArt", false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
    char * dummyArgs[1] { "SimpleQueueReader" };
#pragma GCC diagnostic pop
    MPI_Comm_rank(local_group_comm_, &sink_rank);
    artdaq::EventStore::ARTFUL_FCN * reader =
      useArt ?
      &artapp :
      &artdaq::simpleQueueReaderApp;
    artdaq::EventStore events(conf_.detectors_,
                              conf_.run_,
                              sink_rank,
                              useArt ? conf_.art_argc_ : 1,
                              useArt ? conf_.art_argv_ : dummyArgs,
                              reader);
    artdaq::RHandles h(sink_buffers_ * std::ceil(1.0 * conf_.sources_ / conf_.sinks_),
                       max_payload_size_words_,
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
        fragments_expected += *pfragment->dataBegin();
      }
      else {
        ++fragments_received;
        events.insert(std::move(pfragment));
      }
    }
    while (sources_sending ||
           (Debug << "Waiting for "
            << fragments_expected - fragments_received
            << " fragments." << flusher,
            fragments_received < fragments_expected));
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

fhicl::ParameterSet
Program::getPset(char const * progname)
{
  std::ostringstream descstr;
  descstr << progname
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
    std::cerr << "Exception from command line processing in " << progname
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
  return pset.get<fhicl::ParameterSet>("daq");
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
