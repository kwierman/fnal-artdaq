#ifndef artdaq_DAQdata_detail_DS50Header_hh
#define artdaq_DAQdata_detail_DS50Header_hh
namespace ds50 {
  namespace detail {
    struct Header;
  }
}

struct ds50::detail::Header {
  typedef uint32_t data_t;

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

  static size_t const size_words = 4ul;
};
#endif /* artdaq_DAQdata_detail_DS50Header_hh */
