#ifndef artdaq_DAQdata_DS50BoardWriter_hh
#define artdaq_DAQdata_DS50BoardWriter_hh
////////////////////////////////////////////////////////////////////////
// DS50BoardWriter
//
// Fragment overlay for writing DS50 boards.
//
////////////////////////////////////////////////////////////////////////

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/FragmentHandle.hh"
#include "artdaq/DAQdata/DS50Board.hh"
#include "artdaq/DAQdata/detail/DS50Header.hh"

namespace ds50 {
  class BoardWriter;
}

// Note the inheritance: order is important here (for construction
// initialization order).
class ds50::BoardWriter: artdaq::FragmentHandle, public ds50::Board {
public:
  BoardWriter(artdaq::Fragment &);

  // These functions form overload sets with const functions from
  // ds50::Board.
  adc_type * dataBegin();
  adc_type * dataEnd();

// TODO: Render consistent with naming for accessors, one way or the
// other. Note Fragment getters / setters should be taken into account
// also.
  void setChannelMask(channel_mask_t mask);
  void setPattern(pattern_t pattern);
  void setBoardID(board_id_t id);
  void setEventCounter(event_counter_t event_counter);
  void setTriggerTimeTag(trigger_time_tag_t tag);

  void resize(size_t nAdcs);

private:
  size_t calc_event_size_words_(size_t nAdcs);

  static size_t adcs_to_words_(size_t nAdcs);
  static size_t words_to_frag_words_(size_t nWords);

  detail::Header * header_();
};

inline
ds50::BoardWriter::
BoardWriter(artdaq::Fragment & frag)
  :
  artdaq::FragmentHandle(frag),
  Board(frag_)
{
}

inline
ds50::adc_type *
ds50::BoardWriter::
dataBegin()
{
  assert(frag_.dataSize() > words_to_frag_words_(header_size_words()));
  return reinterpret_cast<adc_type *>(header_() + 1);
}

inline
ds50::adc_type *
ds50::BoardWriter::
dataEnd()
{
  return dataBegin() + total_adc_values();
}

inline
void
ds50::BoardWriter::
setChannelMask(channel_mask_t mask)
{
  header_()->channel_mask = mask;
}

inline
void
ds50::BoardWriter::
setPattern(pattern_t pattern)
{
  header_()->pattern = pattern;
}

inline
void
ds50::BoardWriter::
setBoardID(board_id_t id)
{
  header_()->board_id = id;
  frag_.setFragmentID(id);
}

inline
void
ds50::BoardWriter::
setEventCounter(event_counter_t event_counter)
{
  header_()-> event_counter = event_counter;
  frag_.setSequenceID(event_counter);
}

inline
void
ds50::BoardWriter::
setTriggerTimeTag(trigger_time_tag_t tag)
{
  header_()->trigger_time_tag = tag;
}

inline
void
ds50::BoardWriter::
resize(size_t nAdcs)
{
  auto es(calc_event_size_words_(nAdcs));
  frag_.resize(words_to_frag_words_(es));
  header_()->event_size = es;
}

inline
size_t
ds50::BoardWriter::
calc_event_size_words_(size_t nAdcs)
{
  return adcs_to_words_(nAdcs) + header_size_words();
}

inline
size_t
ds50::BoardWriter::
adcs_to_words_(size_t nAdcs)
{
  auto mod(nAdcs % adcs_per_word_());
  return (mod == 0) ?
    nAdcs / adcs_per_word_() :
    nAdcs / adcs_per_word_() + 1;
}

inline
size_t
ds50::BoardWriter::
words_to_frag_words_(size_t nWords)
{
  size_t mod = nWords % words_per_frag_word_();
  return mod ?
    nWords / words_per_frag_word_() + 1 :
    nWords / words_per_frag_word_();
}

inline
ds50::detail::Header *
ds50::BoardWriter::header_()
{
  assert(frag_.dataSize() >= words_to_frag_words_(header_size_words()));
  return reinterpret_cast<detail::Header *>(&*frag_.dataBegin());
}

#endif /* artdaq_DAQdata_DS50BoardWriter_hh */
