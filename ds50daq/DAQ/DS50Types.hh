#ifndef ds50daq_DAQ_DS50Types_hh
#define ds50daq_DAQ_DS50Types_hh

#include <cstddef>

extern "C" {
#include <stdint.h>
}

#include <vector>

// DS50-specific typedefs
namespace ds50 {
  typedef uint16_t adc_type;
  typedef std::vector<adc_type> ADCCountVec;

  typedef uint64_t reg_type;
  typedef std::vector<reg_type> DataVec;

  typedef double signal_type;
  typedef std::vector<signal_type> SignalVec;
}
#endif /* ds50daq_DAQ_DS50Types_hh */
