
#include "Config.hh"
#include "Perf.hh"
#include "Fragment.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdlib>

#include <mpi.h>

#include <sys/types.h>
#include <regex.h>

using namespace std;

static const char* usage = "TotalNodes TotalEvents DetectorsPerNode SourcesPerNode SinksPerNode EventSize EventQueueSize Run";

static void throwUsage(char* argv0, const string& msg)
{
  cerr << argv0 << " " << usage << "\n";
  throw msg;
}

static int getArgTotalNodes(int argc,char* argv[])
{
  if(argc<2) throwUsage(argv[0], "no total_nodes argument");
  return atoi(argv[1]);
}

static int getArgTotalEvents(int argc,char* argv[])
{
  if(argc<3) throwUsage(argv[0], "no total_events argument");
  return atoi(argv[2]);
}

static double getArgDetectorsPerNode(int argc, char* argv[])
{
  if(argc<4) throwUsage(argv[0], "no detectors_per_node argument");
  return atof(argv[3]);
}

static double getArgSourcesPerNode(int argc, char* argv[])
{
  if(argc<5) throwUsage(argv[0], "no sources_per_node argument");
  return atof(argv[4]);
}

static double getArgSinksPerNode(int argc, char* argv[])
{
  if(argc<6) throwUsage(argv[0], "no sinks_per_node argument");
  return atof(argv[5]);
}

static int getArgEventSize(int argc, char* argv[])
{
  if(argc<7) throwUsage(argv[0], "no event_size argument");
  return atoi(argv[6]);
}

static int getArgQueueSize(int argc, char* argv[])
{
  if(argc<8) throwUsage(argv[0], "no event_queue_size argument");
  return atoi(argv[7]);
}

static int getArgRun(int argc, char* argv[])
{
  if(argc<9) throwUsage(argv[0], "no run argument");
  return atoi(argv[8]);
}

static double numSourceNodes(int total_nodes, double sources_per, double dets_per)
{
  return (double)total_nodes / (sources_per / dets_per + 1);
}

static std::string getProcessorName()
{
  char buf[100];
  int sz=sizeof(buf);
  MPI_Get_processor_name(buf,&sz);
  return std::string(buf);
}

static int getWorkerCount()
{
  char line[250];
  int tot=0;
#if 0
  fstream inp("/proc/cpuinfo");
  if(inp.fail() || inp.bad()) return 1;
  while(true)
    {
      inp.getline(line,sizeof(line));
      if(inp.eof()) break;
      // do regex match here for "processor" in the line
      string strline(line);
      if(strline.find("processor")!=string::npos) ++tot;
    }
#endif
  return tot;
}

// remember rank starts at zero
//run_time_(getArgRuntime(argc,argv)),

Config::Config(int rank, int total_procs, int argc, char* argv[]):
  rank_(rank),
  total_procs_(total_procs),

  total_nodes_(getArgTotalNodes(argc,argv)),
  detectors_per_node_(getArgDetectorsPerNode(argc,argv)),
  sources_per_node_(getArgSourcesPerNode(argc,argv)),
  sinks_per_node_(getArgSinksPerNode(argc,argv)),
  workers_per_node_(getWorkerCount()),

  builder_nodes_((int)numSourceNodes(total_nodes_,sources_per_node_,detectors_per_node_)),
  detector_nodes_(total_nodes_ - builder_nodes_),

  sources_(builder_nodes_ * (int)sources_per_node_),
  sinks_(builder_nodes_ * (int)sinks_per_node_),
  detectors_(sources_),

  detector_start_(0),
  source_start_(detectors_),
  sink_start_(detectors_ + sources_),

  total_events_(getArgTotalEvents(argc,argv)),
  event_size_(getArgEventSize(argc,argv)),
  event_queue_size_(getArgQueueSize(argc,argv)),
  run_(getArgRun(argc,argv)),

  packet_size_(event_size_ / sources_),
  fragment_words_(packet_size_ / sizeof(ElementType)),
  source_buffer_count_(event_queue_size_ * sinks_),
  sink_buffer_count_(event_queue_size_ * sources_),
  type_((rank_<detectors_)?TaskDetector:((rank_<(detectors_+sources_))?TaskSource:TaskSink)),
  offset_(rank_-((type_==TaskDetector)?detector_start_:(type_==TaskSource)?source_start_:sink_start_)),
  barrier_period_(source_buffer_count_),
  node_name_(getProcessorName())
{
	int total_workers = (detectors_+sinks_+sources_);
	if(total_procs_ != total_workers)
	{
		cerr << "total_procs " << total_procs_ << " != "
			 << "total_workers " << total_workers << "\n";
		throw "total_procs != total_workers";
	}
}

int Config::totalReceiveFragments() const
{
#if 1
  return total_events_;
#else
  if(type_==TaskDetector) throw "detectors do not receive events";

  int m = total_events_ / destCount(); // each to receive
  int r = total_events_ % destCount(); // leftovers
  return m + (offset_<r)?1:0;
#endif
}

void Config::writeInfo() const
{
  string fname = infoFilename("config_");
  ofstream ostr(fname.c_str());
  
  if(rank_==0)
    {
      printHeader(ostr);
    }
  ostr << *this << "\n";
}

std::string Config::infoFilename(std::string const& prefix) const
{
  ostringstream ost;
  ost << prefix << setfill('0') << setw(4) << run_ << "_" << setfill('0') << setw(4) << rank_ << ".txt";
  return ost.str();
}

int Config::destCount() const
{
  if(type_ == TaskSink) throw "No destCount for a sink";
  return type_==TaskDetector?sources_:sinks_;
}

int Config::destStart() const
{
  if(type_ == TaskSink) throw "No destStart for a sink";
  return type_==TaskDetector?source_start_:sink_start_;
}

int Config::srcCount() const
{
  if(type_ == TaskDetector) throw "No srcCount for a detector";
  return type_==TaskSink?sources_:detectors_;
}

int Config::srcStart() const
{
  if(type_ == TaskDetector) throw "No srcStart for a detector";
  return type_==TaskSink?source_start_:detector_start_;
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

void Config::printHeader(std::ostream& ost) const 
{
  ost << "Rank TotalNodes "
      << "DetectorsPerNode SourcesPerNode SinksPerNode "
      << "BuilderNodes DetectorNodes Sources Sinks Detectors "
      << "DetectorStart SourceStart SinkStart "
      << "TotalEvents TotalReceiveFragments "
      << "EventSize EventQueueSize "
      << "Run PacketSize "
      << "SourceBufferCount SinkBufferCount "
      << "Type Offset "
      << "BarrierPeriod "
      << "Nodename "
	  << "StartTime\n";
}

void Config::print(std::ostream& ost) const
{
  ost << rank_ << " "
      << total_nodes_ << " "
      << detectors_per_node_ << " "
      << sources_per_node_ << " "
      << sinks_per_node_ << " "
      << builder_nodes_ << " "
      << detector_nodes_ << " "
      << sources_ << " "
      << sinks_ << " "
      << detectors_ << " "
      << detector_start_ << " "
      << source_start_ << " "
      << sink_start_ << " "
      << total_events_ << " "
      << totalReceiveFragments() << " "
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
