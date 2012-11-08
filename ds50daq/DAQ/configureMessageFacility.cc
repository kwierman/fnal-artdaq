#include "ds50daq/DAQ/configureMessageFacility.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

void ds50daq::configureMessageFacility(char const* progname)
{
  mf::StartMessageFacility(mf::MessageFacilityService::MultiThread,
                           mf::MessageFacilityService::logConsole());
  mf::SetModuleName(progname);
  mf::SetContext(progname);
}
