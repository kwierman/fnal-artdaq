#include <iostream>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq/Application/configureMessageFacility.hh"
#include "artdaq/Application/TaskType.hh"
#include "artdaq/Application/MPI2/EventBuilderApp.hh"
#include "artdaq/ExternalComms/xmlrpc_commander.hh"
#include "artdaq/Application/MPI2/MPISentry.hh"
#include "artdaq/DAQrate/quiet_mpi.hh"

int main(int argc, char *argv[])
{
  // initialization
  int const wanted_threading_level { MPI_THREAD_MULTIPLE };
  artdaq::MPISentry mpiSentry(&argc, &argv, wanted_threading_level);
  artdaq::configureMessageFacility("eventbuilder");
  mf::LogDebug("EventBuilder::main")
    << "MPI initialized with requested thread support level of "
    << wanted_threading_level << ", actual support level = "
    << mpiSentry.threading_level() << ".";
  mf::LogDebug("EventBuilder::main")
    << "size = "
    << mpiSentry.procs()
    << ", rank = "
    << mpiSentry.rank();


 // set up an MPI communication group with other EventBuilders
  MPI_Comm local_group_comm;
  int status =
    MPI_Comm_split(MPI_COMM_WORLD, artdaq::TaskType::EventBuilderTask, 0,
                   &local_group_comm);
  if (status == MPI_SUCCESS) {
    int temp_rank;
    MPI_Comm_rank(local_group_comm, &temp_rank);
    mf::LogDebug("EventBuilder")
      << "Successfully created local communicator for type "
      << artdaq::TaskType::EventBuilderTask << ", identifier = 0x"
      << std::hex << local_group_comm << std::dec
      << ", rank = " << temp_rank << ".";
  }
  else {
    mf::LogError("EventBuilder")
      << "Failed to create the local MPI communicator group for "
      << "EventBuilders, status code = " << status << ".";
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

  artdaq::setMsgFacAppName("EventBuilder", vm["port"].as<unsigned short> ()); 

  // create the EventBuilderApp
  artdaq::EventBuilderApp evb_app(mpiSentry.rank(), local_group_comm);;

  // create the xmlrpc_commander and run it
  xmlrpc_commander commander(vm["port"].as<unsigned short> (), evb_app);
  commander.run();
}
