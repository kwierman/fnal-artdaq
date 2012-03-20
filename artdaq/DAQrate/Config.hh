#ifndef artdaq_DAQrate_Config_hh
#define artdaq_DAQrate_Config_hh

#include <iostream>
#include <string>

// sources are first, sinks are second
// the offset_ is the index of the first sink
// the offset_ = sources_

// Define the environment variable ARTDAQ_DAQRATE_USE_ART to any value
// to set use_artapp_ to true.

class Config
{
public:
  enum TaskType { TaskSink=0, TaskSource=1, TaskDetector=2 };

  Config(int rank, int nprocs, int argc, char* argv[]);

  int destCount() const;
  int destStart() const;
  int srcCount() const;
  int srcStart() const;
  int getDestFriend() const;
  int getSrcFriend() const;
  int getArtArgc(int argc, char* argv[]) const;
  char** getArtArgv(int argc, char* argv[]) const;
  std::string typeName() const;
  std::string infoFilename(std::string const& prefix) const;
  void writeInfo() const;

  // input parameters
  int rank_;
  int total_procs_;

  int detectors_;
  int sources_;
  int sinks_;
  int detector_start_;
  int source_start_;
  int sink_start_;

  int event_size_;
  int event_queue_size_;
  int run_;

  // calculated parameters
  int packet_size_;
  int max_initial_send_words_;
  int source_buffer_count_;
  int sink_buffer_count_;
  TaskType type_;
  int offset_;
  int barrier_period_;
  std::string node_name_;
  std::string data_dir_;

  int  art_argc_;
  char** art_argv_;
  bool use_artapp_;

  void print(std::ostream& ost) const;
  void printHeader(std::ostream& ost) const;
};

inline std::ostream& operator<<(std::ostream& ost, Config const& c)
{
  c.print(ost);
  return ost;
}

#endif /* artdaq_DAQrate_Config_hh */
