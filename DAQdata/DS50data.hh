#ifndef artdaq_DAQdata_DS50data_hh
#define artdaq_DAQdata_DS50data_hh

#include "Fragment.hh"
#include "features.hh"

namespace ds50
{

  namespace detail
  {

    struct Header
    {
      typedef uint32_t event_size_t;
      typedef uint8_t channel_mask_t;
      typedef uint16_t pattern_t;
      typedef uint8_t  board_id_t;
      typedef uint32_t event_counter_t;
      typedef uint32_t trigger_time_tag_t;

      uint32_t event_size : 28;
      uint32_t unused_1   :  4;

      uint32_t channel_mask :  8;
      uint32_t pattern      : 16;
      uint32_t unused_2     :  3;
      uint32_t board_id     :  5;

      uint32_t event_counter : 24;
      uint32_t reserved      :  8;

      uint32_t trigger_time_tag : 32;
    };
  }

  class Board
  {
  public:
    typedef detail::Header::event_size_t event_size_t;
    typedef detail::Header::channel_mask_t channel_mask_t;
    typedef detail::Header::pattern_t pattern_t;
    typedef detail::Header::board_id_t board_id_t;
    typedef detail::Header::event_counter_t event_counter_t;
    typedef detail::Header::trigger_time_tag_t trigger_time_tag_t;

    Board();

#if USE_MODERN_FEATURES
    // Make a Board, by taking over the innards of the given Fragment.
    explicit Board(artdaq::Fragment&& f);

    size_t event_size() const;
    channel_mask_t channel_mask() const;
    pattern_t pattern() const;
    board_id_t board_id() const;
    event_counter_t event_counter() const;
    trigger_time_tag_t trigger_time_tag() const;
#endif

  private:
    detail::Header const* header_() const;
    artdaq::Fragment data_;
  };

#if USE_MODERN_FEATURES

  inline
  detail::Header const* Board::header_() const
  { return reinterpret_cast<detail::Header const*>(&*data_.dataBegin()); }

  inline
  Board::Board(artdaq::Fragment&& f) : data_(std::move(f))
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

namespace artdaq
{


  struct DarkSideHeader
  {
    uint32_t word0;
    uint32_t word1;
  };

  struct CompressedBoard
  {
    DarkSideHeader header_;
    CompressedFragParts parts_;
  };
}

#endif
