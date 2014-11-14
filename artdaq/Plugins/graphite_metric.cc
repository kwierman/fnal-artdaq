// graphite_metric.cc: Graphite Metric Plugin
// Author: Eric Flumerfelt
// Last Modified: 11/13/2014
//
// An implementation of the MetricPlugin for Graphite

#include "artdaq/Plugins/MetricMacros.hh"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <ctime>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace artdaq {
  class GraphiteMetric : public MetricPlugin {
  private:
    std::string host_;
    int port_;
    std::string namespace_;
    boost::asio::io_service io_service_;
    tcp::socket socket_;
  public:
    GraphiteMetric(fhicl::ParameterSet config) : MetricPlugin(config),
						 host_(pset.get<std::string>("host","localhost")),
                                                 port_(pset.get<int>("port",2003)),
                                                 namespace_(pset.get<std::string>("namespace","artdaq.")),
                                                 io_service_(),
                                                 socket_(io_service_)
    {
      startMetrics();
    }
    ~GraphiteMetric() { stopMetrics(); }
    virtual std::string getLibName() { return "graphite"; }

    virtual void sendMetric(std::string name, std::string value, std::string unit ) 
    {
      std::string unitWarn = unit;
      const std::time_t result = std::time(0);
      boost::asio::streambuf data;
      std::ostream out(&data);
      out << namespace_ << name << " "
          << value << " "
          << result << std::endl;
   
      boost::asio::write(socket_, data);
          
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
    virtual void sendMetric(std::string name, unsigned long int value, std::string unit ) 
    { 
      sendMetric(name, std::to_string(value), unit);
    }
    virtual void startMetrics() {
      tcp::resolver resolver(io_service_);
      tcp::resolver::query query(host_, std::to_string(port_));
      boost::asio::connect(socket_, resolver.resolve(query));
    }
    virtual void stopMetrics() {
      socket_.shutdown(boost::asio::socket_base::shutdown_both);
      socket_.close();
    }
  };

} //End namespace artdaq

DEFINE_ARTDAQ_METRIC(artdaq::GraphiteMetric)
