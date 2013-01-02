#include <iostream>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "ds50daq/DAQ/configureMessageFacility.hh"
#include "ds50daq/DAQ/BoardReaderApp.hh"
#include "ds50daq/DAQ/xmlrpc_commander.hh"
#include "artdaq/DAQrate/quiet_mpi.hh"

int main(int argc, char *argv[])
{
  // initialization
  ds50::configureMessageFacility("boardreader"); 
  int threading_result;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &threading_result);
  mf::LogDebug("BoardReader::main")
    << "MPI initialized with requested thread support level of "
    << MPI_THREAD_FUNNELED << ", actual support level = "
    << threading_result << ".";
  int procs_;
  int rank_;
  MPI_Comm_size(MPI_COMM_WORLD, &procs_);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  mf::LogDebug("BoardReader::main") << "size = " << procs_ << ", rank = " << rank_;

  // handle the command-line arguments
  std::string usage = std::string(argv[0]) + " -p port_number <other-options>";
  boost::program_options::options_description desc(usage);

  desc.add_options ()
    ("port,p", boost::program_options::value<unsigned short>(), "Port number")
    ("help,h", "produce help message");
  
  boost::program_options::variables_map vm;
  try {
    boost::program_options::store (boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    boost::program_options::notify (vm);
  } catch (boost::program_options::error const& e) {
    mf::LogError ("Option") << "exception from command line processing in " << argv[0] << ": " << e.what() << std::endl;
    return 1;
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  if (!vm.count("port")) {
    mf::LogError ("Option") << argv[0] << " port number not suplied" << std::endl << "For usage and an options list, please do '" << argv[0] <<  " --help'" << std::endl;
    return 1;
  }

  mf::SetApplicationName("BoardReader-" + boost::lexical_cast<std::string>(vm["port"].as<unsigned short> ()));

  // create the BoardReaderApp
  ds50::BoardReaderApp br_app;

  // create the xmlrpc_commander and run it
  xmlrpc_commander commander(vm["port"].as<unsigned short> (), br_app);
  commander.run();

  //xmlrpc http://localhost:5454/RPC2 ds50.init "daq: {event_building_buffer_count:10 max_fragment_size_words: 524288 fragment_receiver: {generator:V172xSimulator freqs_file: \"/home/biery/nov2012/ds50daq/ds50daq/DAQ/ds50_hist.dat\" run_number: 0 events_to_generate: 3 first_event_builder_rank: 1 event_builder_count: 1} event_builder: {first_fragment_receiver_rank: 0 fragment_receiver_count: 1}}"
  //xmlrpc http://localhost:5454/RPC2 ds50.start 101
  //xmlrpc http://localhost:5454/RPC2 ds50.stop

  // cleanup
  MPI_Finalize();
}
