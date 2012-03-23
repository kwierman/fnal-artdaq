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
class Program : MPIProg {
public:
  Program(int argc, char * argv[]);

  void go();
  void source();
  void sink();
  void detector();

private:
  void printHost(const std::string & functionName) const;

  Config conf_;
  MPI_Comm detector_comm_;
  fhicl::ParameterSet daq_control_ps_;
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
  daq_control_ps_()
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
  daq_control_ps_ = pset.get<fhicl::ParameterSet>("daq_control_ps");
  PerfConfigure(conf_, 0); // Don't know how many events.
}

void Program::go()
{
  MPI_Barrier(MPI_COMM_WORLD);
  PerfSetStartTime();
  PerfWriteJobStart();
  int * detRankPtr = new int[conf_.detectors_];
  for (int idx = 0; idx < conf_.detectors_; ++idx) {
    detRankPtr[idx] = idx + conf_.detector_start_;
  }
  MPI_Group orig_group, new_group;
  MPI_Comm_group(MPI_COMM_WORLD, &orig_group);
  MPI_Group_incl(orig_group, conf_.detectors_, detRankPtr, &new_group);
  MPI_Comm_create(MPI_COMM_WORLD, new_group, &detector_comm_);
  delete[] detRankPtr;
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
  artdaq::RHandles from_d(conf_.source_buffer_count_,
                          conf_.max_initial_send_words_,
                          1, // Direct.
                          conf_.getSrcFriend());
  artdaq::SHandles to_r(conf_.sink_buffer_count_,
                        conf_.max_initial_send_words_,
                        conf_.sinks_,
                        conf_.sink_start_);
  artdaq::Fragment::type_t frag_type;
  do {
    from_d.recvEvent(frag);
    frag_type = frag.type();
    to_r.sendEvent(frag);
  }
  while (frag_type != artdaq::Fragment::type_t::END_OF_DATA);
  Debug << "source waiting " << conf_.rank_ << flusher;
  to_r.waitAll();
  from_d.waitAll();
  Debug << "source done " << conf_.rank_ << flusher;
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::detector()
{
  printHost("detector");
  int detector_rank;
  // Should be zero-based, detectors only.
  MPI_Comm_rank(detector_comm_, &detector_rank);
  std::ostringstream det_ps_name_loc;
  det_ps_name_loc << "detectors["
                  << detector_rank
                  << ']';
  std::string det_ps_name;
  if (!(daq_control_ps_.get_if_present(det_ps_name_loc.str(), det_ps_name) ||
        daq_control_ps_.get_if_present("detectors[0]", det_ps_name))) {
    throw cet::exception("Configuration")
        << "Unable to find detector parameter set.";
  }
  fhicl::ParameterSet det_ps =
    daq_control_ps_.get<fhicl::ParameterSet>(det_ps_name);
  std::unique_ptr<artdaq::FragmentGenerator> const gen(make_generator(det_ps));
  artdaq::SHandles h(conf_.source_buffer_count_,
                     conf_.max_initial_send_words_,
                     1, // Direct.
                     conf_.getDestFriend());
  std::cout << "Detector " << conf_.rank_ << " ready." << std::endl;
  MPI_Barrier(detector_comm_);
  // MPI_Barrier(MPI_COMM_WORLD);
  // not using the run time method
  // TimedLoop tl(conf_.run_time_);
  size_t fragments_per_source = -1;
  daq_control_ps_.get_if_present("fragments_per_source", fragments_per_source);
  artdaq::FragmentPtrs frags;
  size_t fragments_sent = 0;
  while (gen->getNext(frags) && fragments_per_source) {
    for (auto & fragPtr : frags) {
      h.sendEvent(*fragPtr);
      --fragments_per_source;
      if (!fragments_per_source) { break; }
      if ((++fragments_sent % 100) == 0) {
        // Don't get too far out of sync.
        MPI_Barrier(detector_comm_);
      }
    }
    frags.clear();
  }
  artdaq::Fragment eod_frag(artdaq::Fragment::InvalidEventID,
                            artdaq::Fragment::InvalidFragmentID,
                            artdaq::Fragment::type_t::END_OF_DATA);
  h.sendEvent(eod_frag);
  Debug << "detector waiting " << conf_.rank_ << flusher;
  h.waitAll();
  Debug << "detector done " << conf_.rank_ << flusher;
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::sink()
{
  printHost("sink");
  {
    // This scope exists to control the lifetime of 'events'
    artdaq::EventStore events(conf_.detectors_,
                              conf_.run_,
                              conf_.art_argc_,
                              conf_.art_argv_);
    artdaq::FragmentPtr pfragment(new artdaq::Fragment);
    artdaq::RHandles h(conf_.sink_buffer_count_,
                       conf_.max_initial_send_words_,
                       conf_.sources_,
                       conf_.source_start_);
    size_t sources_sending = conf_.sources_;
    do {
      h.recvEvent(*pfragment);
      if (pfragment->type() == artdaq::Fragment::type_t::END_OF_DATA) {
        --sources_sending;
      }
      else {
        events.insert(std::move(pfragment));
      }
    }
    while (sources_sending);

    // Now we are done collecting fragments, so we can shut down the
    // receive handles.
    h.waitAll();

    // Make the reader application finish, and capture it's return
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
  std::cout << "Running " << functionName
            << " on host " << hostString
            << " with rank " << rank_ << "." << std::endl;
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
