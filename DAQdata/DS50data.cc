#include "DS50data.hh"

namespace ds50
{
  Board::Board(artdaq::Fragment&& f) : 
    data_(f) 
  { }
}
