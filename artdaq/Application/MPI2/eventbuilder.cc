#include <iostream>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq/Application/configureMessageFacility.hh"
#include "artdaq/Application/MPI2/EventBuilderApp.hh"
#include "artdaq/ExternalComms/xmlrpc_commander.hh"
#include "artdaq/DAQrate/quiet_mpi.hh"

int main(int argc, char *argv[])
{
  // initialization
  artdaq::configureMessageFacility("eventbuilder");
  int threading_result;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &threading_result);
  mf::LogDebug("EventBuilder::main")
    << "MPI initialized with requested thread support level of "
    << MPI_THREAD_MULTIPLE << ", actual support level = "
    << threading_result << ".";
  int procs_;
  int rank_;
  MPI_Comm_size(MPI_COMM_WORLD, &procs_);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  mf::LogDebug("EventBuilder::main") << "size = " << procs_ << ", rank = " << rank_;

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

  mf::SetApplicationName("EventBuilder-" + boost::lexical_cast<std::string>(vm["port"].as<unsigned short> ()));

  // create the EventBuilderApp
  artdaq::EventBuilderApp evb_app(rank_);;

  // create the xmlrpc_commander and run it
  xmlrpc_commander commander(vm["port"].as<unsigned short> (), evb_app);
  commander.run();

  // cleanup
  MPI_Finalize();
}
