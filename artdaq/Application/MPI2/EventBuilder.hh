#ifndef artdaq_Application_MPI2_EventBuilder_hh
#define artdaq_Application_MPI2_EventBuilder_hh

#include <string>
#include <vector>

#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/RunID.h"
#include "artdaq/DAQrate/quiet_mpi.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"
#include "artdaq/DAQrate/RHandles.hh"
#include "artdaq/DAQrate/EventStore.hh"

namespace artdaq
{
  class EventBuilder;
}

class artdaq::EventBuilder
{
public:
  EventBuilder(int mpi_rank);
  EventBuilder(EventBuilder const&) = delete;
  ~EventBuilder();
  EventBuilder& operator=(EventBuilder const&) = delete;

  bool initialize(fhicl::ParameterSet const&);
  bool start(art::RunID);
  bool stop();
  bool pause();
  bool resume();
  bool shutdown();
  bool soft_initialize(fhicl::ParameterSet const&);
  bool reinitialize(fhicl::ParameterSet const&);

  size_t process_fragments();

  std::string report(std::string const&) const;

private:
  void initializeEventStore();

  int mpi_rank_;
  bool local_group_defined_;
  MPI_Comm local_group_comm_;

  std::string init_string_;
  fhicl::ParameterSet previous_pset_;

  uint64_t max_fragment_size_words_;
  size_t mpi_buffer_count_;
  size_t first_data_sender_rank_;
  size_t data_sender_count_;
  size_t expected_fragments_per_event_;
  size_t eod_fragments_received_;
  bool use_art_;
  bool print_event_store_stats_;
  art::RunID run_id_;

  std::unique_ptr<artdaq::RHandles> receiver_ptr_;
  std::unique_ptr<artdaq::EventStore> event_store_ptr_;
  bool art_initialized_;

  /* This is used for syncronization between the thread running 
     process_fragments() and XMLRPC calls.  This will be locked before data
     readout begins by the start() and resume() methods in the event builder.
     It will be unlocked by the process_fragments() thread once EOD fragments
     and all data has been received.  The stop() and pause() methods will
     attempt to lock the mutex as well and will be blocked until all data has
     been clocked into the EventBuilder. */
  std::mutex flush_mutex_;
};

#endif /* artdaq_Application_MPI2_EventBuilder_hh */