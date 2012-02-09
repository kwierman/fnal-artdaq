
#include "Config.hh"
#include "Perf.hh"
#include "Utils.hh"
#include "FragmentPool.hh"
#include "EventStore.hh"
#include "RHandles.hh"
#include "SHandles.hh"
#include "Debug.hh"

#include "MPIProg.hh"

#include <math.h>
#include <sys/resource.h>

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>
#include <unistd.h>

using namespace std;

// Class Program is our application object.
class Program : MPIProg {
public:
  Program(int argc, char* argv[]);

  void go();
  void source();
  void sink();
  void detector();

private:
  void printHost(const std::string & functionName) const;

  Config conf_;
  MPI_Comm detector_comm_;
};

Program::Program(int argc, char* argv[]):
  MPIProg(argc, argv),
  conf_(rank_, procs_, argc, argv)
{
  PerfConfigure(conf_);
  conf_.writeInfo();
  configureDebugStream(conf_.rank_, conf_.run_);
}

void Program::go()
{
  MPI_Barrier(MPI_COMM_WORLD);
  PerfSetStartTime();
  PerfWriteJobStart();
  int* detRankPtr = new int[conf_.detectors_];
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
  RHandles from_d(conf_);
  SHandles to_r(conf_);
  for (int i = 0; i < conf_.total_events_; ++i) {
    from_d.recvEvent(frag);
    to_r.sendEvent(frag);
  }
  Debug << "source waiting " << conf_.rank_ << flusher;
  to_r.waitAll();
  from_d.waitAll();
  Debug << "source done " << conf_.rank_ << flusher;
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::detector()
{
  printHost("detector");
  FragmentPool pool(conf_);
  artdaq::Fragment frag;
  SHandles h(conf_);
  std::cout << "Detector " << conf_.rank_ << " ready." << std::endl;
  MPI_Barrier(detector_comm_);
  // MPI_Barrier(MPI_COMM_WORLD);
  // not using the run time method
  // TimedLoop tl(conf_.run_time_);
  for (int i = 0; i < conf_.total_events_; ++i) {
    if ((i % 100) == 0) {
      MPI_Barrier(detector_comm_);
    }
    pool(frag); // read or generate fragment
    h.sendEvent(frag);
  }
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
    artdaq::EventStore events(conf_);
    artdaq::FragmentPtr pfragment(new artdaq::Fragment);
    RHandles h(conf_);
    int total_events = conf_.total_events_ / conf_.sinks_;
    if (conf_.offset_ < (conf_.total_events_ % conf_.sinks_)) { ++total_events; }
    int expect = total_events * conf_.sources_;
    // MPI_Barrier(MPI_COMM_WORLD);
    // remember that we get one event fragment from each source
    Debug << "expect=" << expect << flusher;
    Debug << "total_events=" << total_events << flusher;
    for (int i = 0; i < expect; ++i) {
      h.recvEvent(*pfragment);
      events.insert(std::move(pfragment));
    }
    events.endOfData();
    h.waitAll(); // not sure if this should be inside braces
  } // end of lifetime of 'events'
  Debug << "Sink done " << conf_.rank_ << flusher;
  MPI_Barrier(MPI_COMM_WORLD);
}

void Program::printHost(const std::string & functionName) const
{
  char* doPrint = getenv("PRINT_HOST");
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

// ---------------

int main(int argc, char* argv[])
{
  int rc = 1;
  try {
    Program p(argc, argv);
    p.go();
    rc = 0;
  }
  catch (string & s) {
    cerr << "yuck - " << s << "\n";
  }
  catch (const char* c) {
    cerr << "yuck - " << c << "\n";
  }
  return rc;
}


void printUsage()
{
  int myid = 0;
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  cout << myid << ":"
       << " user=" << asDouble(usage.ru_utime)
       << " sys=" << asDouble(usage.ru_stime)
       << endl;
}

