#include "Fragment.hh"

namespace artdaq
{
  Fragment::Fragment() : vals_(data_offset_(), 0)
  { }

  Fragment::Fragment(std::size_t n) : vals_(n+data_offset_(), 0)
  { }
}
