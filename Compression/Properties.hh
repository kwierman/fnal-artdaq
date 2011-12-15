#ifndef COMP_PROPERTIES_H
#define COMP_PROPERTIES_H

/*
  domain is [-20,140] mV, range is [0,2^12] counts
  points are (-20,0) and (140,2^12)

  look up attributes and constexpr in C++11 as a better way to do this
*/

#include <vector>
#include <cstddef>

typedef unsigned short adc_type;
typedef std::vector<adc_type> ADCCountVec;

typedef unsigned long reg_type;
typedef std::vector<reg_type> DataVec;

typedef double signal_type;
typedef std::vector<signal_type> SignalVec;

constexpr unsigned long reg_size_bits = (sizeof(reg_type)*8);
constexpr unsigned long chunk_size_bytes = 1<<16;
constexpr unsigned long chunk_size_counts = chunk_size_bytes / sizeof(adc_type);
constexpr unsigned long chunk_size_regs = chunk_size_bytes / sizeof(reg_type);

inline size_t bitCountToBytes(reg_type bits)
{
  return ( bits/reg_size_bits + ((bits%reg_size_bits)==0 ? 0 : 1 )) * sizeof(reg_type);
}

template <size_t N>
struct Properties_t
{
  constexpr static adc_type count_max() { return 1<<N; }
  constexpr static adc_type count_min() { return 0; }

  constexpr static double signal_low() { return -20.; }
  constexpr static double signal_high() { return 140.; }
  constexpr static double adc_low() { return count_min(); }
  constexpr static double adc_high() { return (double)(count_max()); }

  constexpr static double m() { return (adc_high()-adc_low())/(signal_high()-signal_low()); }
  constexpr static double b() { return -signal_low() * m(); }

  static adc_type signalToADC(double sig)
  { 
    double x = sig<signal_low() ? signal_low() : sig>signal_high() ? signal_high() : sig;
    return (adc_type)(m()*x + b()); 
  }

  static double ADCToSignal(adc_type counts)
  {
    return ((double)counts-b())/m();
  }

};

typedef Properties_t<12> Properties;

#endif
