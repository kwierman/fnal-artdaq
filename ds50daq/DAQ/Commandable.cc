#include "ds50daq/DAQ/Commandable.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

void ds50::Commandable::BootedEnter()
{
}

void ds50::Commandable::BootedExit()
{
}

bool ds50::Commandable::initialize()
{
  return true;
}

bool ds50::Commandable::softInitialize()
{
  return true;
}

bool ds50::Commandable::reinitialize()
{
  return true;
}

void ds50::Commandable::InitializedEnter()
{
}

void ds50::Commandable::InitializedExit()
{
}

bool ds50::Commandable::beginRun()
{
  return true;
}

bool ds50::Commandable::endRun()
{
  return true;
}

bool ds50::Commandable::pauseRun()
{
  return true;
}

bool ds50::Commandable::resumeRun()
{
  return true;
}

void ds50::Commandable::invalidTransitionRequest()
{
}
