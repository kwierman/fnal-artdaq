#include "Board.hh"

ds50::Board::Board() :
  data_()
{ }

Board::Board(artdaq::Fragment&& f) : 
  data_(f) 
{ }
