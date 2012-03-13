#ifndef artdaq_DAQdata_DS50Board_hh
#define artdaq_DAQdata_DS50Board_hh

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/detail/DS50Header.hh"
#include "artdaq/DAQdata/features.hh"

// Fragment overlay class for DS50 data.
namespace ds50 {
  class Board {
  public:
    typedef detail::Header::event_size_t event_size_t;
    typedef detail::Header::channel_mask_t channel_mask_t;
    typedef detail::Header::pattern_t pattern_t;
    typedef detail::Header::board_id_t board_id_t;
    typedef detail::Header::event_counter_t event_counter_t;
    typedef detail::Header::trigger_time_tag_t trigger_time_tag_t;

#if USE_MODERN_FEATURES
    // Make a Board, by taking over the innards of the given Fragment.
    explicit Board(artdaq::Fragment & f);

    size_t event_size() const;
    channel_mask_t channel_mask() const;
    pattern_t pattern() const;
    board_id_t board_id() const;
    event_counter_t event_counter() const;
    trigger_time_tag_t trigger_time_tag() const;
#endif

  private:
    detail::Header const * header_() const;
    artdaq::Fragment & data_;
  };

#if USE_MODERN_FEATURES
  inline
  detail::Header const * Board::header_() const
  { return reinterpret_cast<detail::Header const *>(&*data_.dataBegin()); }

  inline
  Board::Board(artdaq::Fragment & f)
    :
    data_(f)
  { }

  inline
  size_t Board::event_size() const
  { return header_()->event_size; }

  Board::channel_mask_t Board::channel_mask() const
  { return header_()->channel_mask; }

  inline
  Board::pattern_t Board::pattern() const
  { return header_()->pattern; }

  inline
  Board::board_id_t Board::board_id() const
  { return header_()->board_id; }

  inline
  Board::event_counter_t Board::event_counter() const
  { return header_()->event_counter; }

  inline
  Board::trigger_time_tag_t Board::trigger_time_tag() const
  { return header_()->trigger_time_tag; }
#endif
}
#endif /* artdaq_DAQdata_DS50Board_hh */
