
#include "xmlrpc_commander.hh"
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "ds50daq/DAQ/configureMessageFacility.hh"

int main(int argc, char *argv[])
{
  ds50::configureMessageFacility("BoardReader::main"); 

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

  //// create the fragment receiver thread
  //ds50::FragmentReceiver fragRec(requestQueue, responseQueue);
  //std::thread frThread(std::bind(&ds50::FragmentReceiver::run, fragRec));

  // create the xmlrpc_commander and run it
  xmlrpc_commander commander(vm["port"].as<unsigned short> ());
  commander.run();

  //// cleanup
  //frThread.join();
}
