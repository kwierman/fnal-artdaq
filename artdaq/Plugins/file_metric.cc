// FileMetric.h: File Metric Plugin
// Author: Eric Flumerfelt
// Last Modified: 11/06/2014
//
// An implementation of the MetricPlugin for Log Files

#include "artdaq/Plugins/MetricMacros.hh"
#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <ctime>
#include <string>

namespace artdaq {
  class FileMetric : public MetricPlugin {
  private:
    std::string outputFile_;
    std::ofstream outputStream_;
  public:
    FileMetric(fhicl::ParameterSet config) : MetricPlugin(config),
				     outputFile_(pset.get<std::string>("fileName","FileMetric.out"))
    {
      std::string modeString = pset.get<std::string>("fileMode", "append");
      
      std::ios_base::openmode mode = std::ofstream::out | std::ofstream::app;
      if(modeString == "Overwrite" || modeString == "Create" || modeString == "Write") {
          mode = std::ofstream::out | std::ofstream::trunc;
      }

      outputStream_.open(outputFile_.c_str(),mode);
    }
    ~FileMetric() {
      outputStream_.close();
    }
    virtual std::string getLibName() { return "file"; }
    virtual void sendMetric(std::string name, std::string value, std::string unit ) 
    {
      const std::time_t result = std::time(NULL);
      outputStream_ << std::ctime(&result) << "FileMetric: " << name << ": " << value << " " << unit << "." << std::endl;
    }
    virtual void sendMetric(std::string name, int value, std::string unit ) 
    { 
      sendMetric(name, std::to_string(value), unit);
    }
    virtual void sendMetric(std::string name, double value, std::string unit ) 
    { 
      sendMetric(name, std::to_string(value), unit);
    }
    virtual void sendMetric(std::string name, float value, std::string unit ) 
    {
      sendMetric(name, std::to_string(value), unit);
    }
    virtual void sendMetric(std::string name, uint32_t value, std::string unit ) 
    { 
      sendMetric(name, std::to_string(value), unit);
    }
  };

} //End namespace artdaq

DEFINE_ARTDAQ_METRIC(artdaq::FileMetric)
