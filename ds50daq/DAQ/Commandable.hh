#ifndef ds50daq_DAQ_Commandable_hh
#define ds50daq_DAQ_Commandable_hh

#include <string>
#include <vector>

#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/RunID.h"

namespace ds50
{
  class Commandable;
}

class ds50::Commandable
{
public:
  Commandable() = default;
  Commandable(Commandable const&) = delete;
  virtual ~Commandable() = default;
  Commandable& operator=(Commandable const&) = delete;

  virtual bool initialize(fhicl::ParameterSet const&) = 0;
  virtual bool pause() = 0;
  virtual bool start(art::RunID) = 0;
  virtual bool resume() = 0;
  virtual bool stop() = 0;
  virtual /* Report_ptr */ std::string report(std::string const& which) const = 0;
  virtual std::string status() const = 0;
  virtual bool perfreset(std::string const& which) = 0;
  virtual bool shutdown() = 0;
  virtual std::vector<std::string> legalCommands() const = 0;
};


#endif
