#ifndef ds50daq_DAQ_configureMessageFacility_hh
#define ds50daq_DAQ_configureMessageFacility_hh

namespace ds50
{
  // Configure and start the message facility. Provide the program
  // name so that messages will be appropriately tagged.
  void configureMessageFacility(char const* progname);
}

#endif
