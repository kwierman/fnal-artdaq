#ifndef artdaq_DAQdata_DS50Board_hh
#define artdaq_DAQdata_DS50Board_hh

#include "artdaq/DAQdata/DS50Types.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/detail/DS50Header.hh"

#include <ostream>

// Fragment overlay class for DS50 data.
namespace ds50 {
  class Board;

  std::ostream & operator << (std::ostream &, Board const &);
}

class ds50::Board {
public:
  typedef detail::Header::data_t data_t;
  typedef detail::Header::event_size_t event_size_t;
  typedef detail::Header::channel_mask_t channel_mask_t;
  typedef detail::Header::pattern_t pattern_t;
  typedef detail::Header::board_id_t board_id_t;
  typedef detail::Header::event_counter_t event_counter_t;
  typedef detail::Header::trigger_time_tag_t trigger_time_tag_t;

  explicit Board(artdaq::Fragment const & f);

  size_t event_size() const;
  channel_mask_t channel_mask() const;
  pattern_t pattern() const;
  board_id_t board_id() const;
  event_counter_t event_counter() const;
  trigger_time_tag_t trigger_time_tag() const;

  size_t total_adc_values() const;
  adc_type const * dataBegin() const;
  adc_type const * dataEnd() const;

  bool fastVerify() const;
  adc_type const * findBadADC() const;
  void checkADCData() const; // Throws on failure.

  static constexpr size_t header_size_words();

private:
  detail::Header const * header_() const;
  artdaq::Fragment const & data_;
};

inline
ds50::Board::Board(artdaq::Fragment const & f)
  :
  data_(f)
{
}

inline
size_t
ds50::Board::event_size() const
{
  return header_()->event_size;
}

inline
ds50::Board::channel_mask_t
ds50::Board::channel_mask() const
{
  return header_()->channel_mask;
}

inline
ds50::Board::pattern_t
ds50::Board::pattern() const
{
  return header_()->pattern;
}

inline
ds50::Board::board_id_t
ds50::Board::board_id() const
{
  return header_()->board_id;
}

inline
ds50::Board::event_counter_t
ds50::Board::event_counter() const
{
  return header_()->event_counter;
}

inline
ds50::Board::trigger_time_tag_t
ds50::Board::trigger_time_tag() const
{
  return header_()->trigger_time_tag;
}

inline
size_t
ds50::Board::total_adc_values() const
{
  return (event_size() - header_size_words()) * 2;
}

inline
ds50::adc_type const *
ds50::Board::dataBegin() const
{
  return reinterpret_cast<adc_type const *>(header_()) + header_size_words() * 2;
}

inline
ds50::adc_type const *
ds50::Board::dataEnd() const
{
  return dataBegin() + total_adc_values();
}

inline
bool
ds50::Board::fastVerify() const
{
  return (findBadADC() == dataEnd());
}

inline
ds50::adc_type const *
ds50::Board::findBadADC() const
{
  return std::find_if(dataBegin(),
                      dataEnd(),
                      [](adc_type const adc)
                      -> bool
  { return (adc >> 12); });
}

inline
constexpr
size_t
ds50::Board::header_size_words()
{
  return detail::Header::size_words;
}

inline
ds50::detail::Header const *
ds50::Board::header_() const
{
  return reinterpret_cast<detail::Header const *>(&*data_.dataBegin());
}
#endif /* artdaq_DAQdata_DS50Board_hh */
