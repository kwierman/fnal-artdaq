#include "artdaq/Application/configureMessageFacility.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

void artdaq::configureMessageFacility(char const* progname)
{
  mf::StartMessageFacility(mf::MessageFacilityService::MultiThread,
                           mf::MessageFacilityService::logConsole());
  mf::SetModuleName(progname);
  mf::SetContext(progname);
}
