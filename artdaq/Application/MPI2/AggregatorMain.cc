#include <iostream>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include "artdaq/Application/TaskType.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq/Application/configureMessageFacility.hh"
#include "artdaq/Application/MPI2/AggregatorApp.hh"
#include "artdaq/ExternalComms/xmlrpc_commander.hh"
#include "artdaq/DAQrate/quiet_mpi.hh"

int main(int argc, char *argv[])
{
  // initialization
  artdaq::configureMessageFacility("aggregator"); 

  int threading_result;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &threading_result);
  mf::LogDebug("Aggregator::main")
    << "MPI initialized with requested thread support level of "
    << MPI_THREAD_FUNNELED << ", actual support level = "
    << threading_result << ".";
  int procs;
  int rank;
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  mf::LogDebug("Aggregator::main") << "size = " << procs << ", rank = " << rank;

  // set up an MPI communication group with other Aggregators
  MPI_Comm local_group_comm;
  int status =
    MPI_Comm_split(MPI_COMM_WORLD, artdaq::TaskType::AggregatorTask, 0,
                   &local_group_comm);
  if (status == MPI_SUCCESS) {
    int temp_rank;
    MPI_Comm_rank(local_group_comm, &temp_rank);
    mf::LogDebug("Aggregator")
      << "Successfully created local communicator for type "
      << artdaq::TaskType::AggregatorTask << ", identifier = 0x"
      << std::hex << local_group_comm << std::dec
      << ", rank = " << temp_rank << ".";
  }
  else {
    mf::LogError("Aggregator")
      << "Failed to create the local MPI communicator group for "
      << "Aggregators, status code = " << status << ".";
    return 3;
  }

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

  artdaq::setMsgFacAppName("Aggregator", vm["port"].as<unsigned short> ()); 

  // create the AggregatorApp
  artdaq::AggregatorApp agg_app(rank, local_group_comm);

  // create the xmlrpc_commander and run it
  xmlrpc_commander commander(vm["port"].as<unsigned short> (), agg_app);
  commander.run();

  // cleanup
  MPI_Finalize();
}
