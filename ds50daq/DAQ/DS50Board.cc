#include "ds50daq/DAQ/DS50Board.hh"

#include "cetlib/exception.h"

void
ds50::Board::checkADCData() const
{
  ds50::adc_type const * adcPtr(findBadADC());
  if (adcPtr != dataEnd()) {
    throw cet::exception("IllegalADCVal")
        << "Illegal value of ADC word #"
        << (adcPtr - dataBegin())
        << ": 0x"
        << std::hex
        << *adcPtr
        << ".";
  }
}

std::ostream &
ds50::operator << (std::ostream & os, Board const & b)
{
  os << "Board ID: "
     << static_cast<uint16_t>(b.board_id())
     << ", event counter: "
     << b.event_counter()
     << ". event size: "
     << b.event_size()
     << "w, channels: "
     << b.total_adc_values()
     << ", trigger time: "
     << b.trigger_time_tag()
     << "\n";
  os << "  Channel mask: "
     << std::hex
     << static_cast<uint16_t>(b.channel_mask())
     << std::dec
     << ", pattern: "
     << b.pattern()
     << ".\n";
  return os;
}
