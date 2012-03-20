
#include "artdaq/DAQrate/Config.hh"
#include "artdaq/DAQrate/Perf.hh"
#include "artdaq/DAQdata/Fragment.hh"

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdlib>

#include "artdaq/DAQrate/quiet_mpi.hh"

#include <sys/types.h>
#include <regex.h>

using namespace std;

static const char* usage = "DetectorsPerNode SinksPerNode EventSize EventQueueSize Run";

static void throwUsage(char* argv0, const string & msg)
{
  cerr << argv0 << " " << usage << "\n";
  throw msg;
}

static double getArgDetectors(int argc, char* argv[])
{
  if (argc < 2) { throwUsage(argv[0], "no detectors_per_node argument"); }
  return atof(argv[1]);
}

static double getArgSinks(int argc, char* argv[])
{
  if (argc < 3) { throwUsage(argv[0], "no sinks_per_node argument"); }
  return atof(argv[2]);
}

static int getArgEventSize(int argc, char* argv[])
{
  if (argc < 4) { throwUsage(argv[0], "no event_size argument"); }
  return atoi(argv[4]);
}

static int getArgQueueSize(int argc, char* argv[])
{
  if (argc < 5) { throwUsage(argv[0], "no event_queue_size argument"); }
  return atoi(argv[5]);
}

static int getArgRun(int argc, char* argv[])
{
  if (argc < 6) { throwUsage(argv[0], "no run argument"); }
  return atoi(argv[6]);
}

static std::string getArgDataDir(int argc, char* argv[])
{
  if (argc < 7) {return std::string();}
  std::string rawArg(argv[7]);
  if (rawArg.find("--data-dir=") == std::string::npos) {return "";}
  std::string dataDir = rawArg.substr(11);
  if (dataDir.length() > 0 && dataDir[dataDir.length() - 1] != '/') {
    dataDir.append("/");
  }
  return dataDir;
}


static std::string getProcessorName()
{
  char buf[100];
  int sz = sizeof(buf);
  MPI_Get_processor_name(buf, &sz);
  return std::string(buf);
}


// remember rank starts at zero
//run_time_(getArgRuntime(argc,argv)),

Config::Config(int rank, int total_procs, int argc, char* argv[]):
  rank_(rank),
  total_procs_(total_procs),

  detectors_(getArgDetectors(argc, argv)),
  sources_(detectors_),
  sinks_(getArgSinks(argc, argv)),

  detector_start_(0),
  source_start_(detectors_),
  sink_start_(detectors_ + sources_),

  event_size_(getArgEventSize(argc, argv)),
  event_queue_size_(getArgQueueSize(argc, argv)),
  run_(getArgRun(argc, argv)),

  packet_size_(event_size_ / sources_),
  max_initial_send_words_(packet_size_ / sizeof(artdaq::RawDataType)),
  source_buffer_count_(event_queue_size_ * sinks_),
  sink_buffer_count_(event_queue_size_ * sources_),
  type_((rank_ < detectors_) ? TaskDetector : ((rank_ < (detectors_ + sources_)) ? TaskSource : TaskSink)),
  offset_(rank_ - ((type_ == TaskDetector) ? detector_start_ : (type_ == TaskSource) ? source_start_ : sink_start_)),
  barrier_period_(source_buffer_count_),
  node_name_(getProcessorName()),
  data_dir_(getArgDataDir(argc, argv)),

  art_argc_(getArtArgc(argc, argv)),
  art_argv_(getArtArgv(argc - art_argc_, argv)),
  use_artapp_(getenv("ARTDAQ_DAQRATE_USE_ART") != 0)

{
  int total_workers = (detectors_ + sinks_ + sources_);
  if (total_procs_ != total_workers) {
    cerr << "total_procs " << total_procs_ << " != "
         << "total_workers " << total_workers << "\n";
    throw "total_procs != total_workers";
  }
}

void Config::writeInfo() const
{
  string fname = infoFilename("config_");
  ofstream ostr(fname.c_str());
  if (rank_ == 0) {
    printHeader(ostr);
  }
  ostr << *this << "\n";
}

std::string Config::infoFilename(std::string const & prefix) const
{
  ostringstream ost;
  ost << prefix << setfill('0') << setw(4) << run_ << "_" << setfill('0') << setw(4) << rank_ << ".txt";
  return ost.str();
}

int Config::destCount() const
{
  if (type_ == TaskSink) { throw "No destCount for a sink"; }
  return type_ == TaskDetector ? sources_ : sinks_;
}

int Config::destStart() const
{
  if (type_ == TaskSink) { throw "No destStart for a sink"; }
  return type_ == TaskDetector ? source_start_ : sink_start_;
}

int Config::srcCount() const
{
  if (type_ == TaskDetector) { throw "No srcCount for a detector"; }
  return type_ == TaskSink ? sources_ : detectors_;
}

int Config::srcStart() const
{
  if (type_ == TaskDetector) { throw "No srcStart for a detector"; }
  return type_ == TaskSink ? source_start_ : detector_start_;
}

std::string Config::typeName() const
{
  static const char* names[] = { "Sink", "Source", "Detector" };
  return names[type_];
}

int Config::getDestFriend() const
{
  return offset_ + destStart();
}

int Config::getSrcFriend() const
{
  return offset_ + srcStart();
}

int Config::getArtArgc(int argc, char* argv[]) const
{
  // Find the '--' in argv
  int pos = 0;
  for (; pos < argc; ++pos) {
    if (strcmp(argv[pos], "--") == 0) { break; }
  }
  return argc - pos;
}

char** Config::getArtArgv(int pos, char** argv) const
{
  return argv + pos;
}

void Config::printHeader(std::ostream & ost) const
{
  ost << "Rank TotalNodes "
      << "DetectorsPerNode SourcesPerNode SinksPerNode "
      << "BuilderNodes DetectorNodes Sources Sinks Detectors "
      << "DetectorStart SourceStart SinkStart "
      << "EventSize EventQueueSize "
      << "Run PacketSize "
      << "SourceBufferCount SinkBufferCount "
      << "Type Offset "
      << "BarrierPeriod "
      << "Nodename "
      << "StartTime\n";
}

void Config::print(std::ostream & ost) const
{
  ost << rank_ << " "
      << sources_ << " "
      << sinks_ << " "
      << detectors_ << " "
      << detector_start_ << " "
      << source_start_ << " "
      << sink_start_ << " "
      << event_size_ << " "
      << event_queue_size_ << " "
      << run_ << " "
      << packet_size_ << " "
      << source_buffer_count_ << " "
      << sink_buffer_count_ << " "
      << typeName() << " "
      << offset_ << " "
      << barrier_period_ << " "
      << node_name_ << " "
      << PerfGetStartTime();
}
