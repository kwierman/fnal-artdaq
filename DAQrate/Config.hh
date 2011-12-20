#ifndef cConfig_HHH
#define cConfig_HHH

#include <iostream>
#include <string>

// sources are first, sinks are second
// the offset_ is the index of the first sink
// the offset_ = sources_

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
  std::string typeName() const;
  int totalReceiveFragments() const;
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

  int total_events_;
  int event_size_;
  int event_queue_size_;
  int run_;

  // calculated parameters
  int packet_size_;
  int fragment_words_;
  int source_buffer_count_;
  int sink_buffer_count_;
  TaskType type_;
  int offset_;
  int barrier_period_;
  std::string node_name_;
  std::string data_dir_;
  char **art_argv_;
  int  art_artc_;

  void print(std::ostream& ost) const;
  void printHeader(std::ostream& ost) const;
};

inline std::ostream& operator<<(std::ostream& ost, Config const& c)
{
  c.print(ost);
  return ost;
}

#endif
