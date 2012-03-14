#include "artdaq/DAQrate/DS50FragmentReader.hh"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "artdaq/DAQdata/DS50Board.hh"
#include "fhiclcpp/ParameterSet.h"

using fhicl::ParameterSet;
using ds50::Board;

artdaq::DS50FragmentReader::DS50FragmentReader(ParameterSet const & ps)
  :
  fileNames_(ps.get<std::vector<std::string>>("fileNames")),
  max_set_size_bytes_(ps.get<double>("max_set_size_gib", 14.0) * 1024 * 1024),
  next_point_ {fileNames_.begin(), 0} {
}

artdaq::DS50FragmentReader::~DS50FragmentReader()
{ }

bool
artdaq::DS50FragmentReader::getNext_(FragmentPtrs & frags)
{
  if (next_point_.first == fileNames_.end()) {
    return false; // Nothing to do..
  }
  // Useful constants for byte arithmetic.
  static size_t const ds50_words_per_frag_word =
    sizeof(RawDataType) /
    sizeof(Board::data_t);
  static size_t const initial_frag_size =
    Board::header_size_words() /
    ds50_words_per_frag_word;
  static size_t const header_size_bytes =
    Board::header_size_words() * sizeof(Board::data_t);
  // Open file.
  std::ifstream in_data;
  uint64_t read_bytes = 0;
  while (!((max_set_size_bytes_ < read_bytes) ||
           next_point_.first == fileNames_.end())) {
    if (!in_data.is_open()) {
      in_data.open((*next_point_.first).c_str(),
                   std::ios::in | std::ios::binary);
      // Find where we left off.
      in_data.seekg(next_point_.second);
    }
    frags.emplace_back(new Fragment(initial_frag_size));
    Fragment & frag = *frags.back();
    // Read DS50 header.
    in_data.read(reinterpret_cast<char *>(&*frag.dataBegin()),
                 header_size_bytes);
    Board board(frag);
    std::cerr << "INFO: Found a DS50 board of size "
              << board.event_size()
              << " words.\n";
    frag.resize((board.event_size() + board.event_size() % 2) /
                ds50_words_per_frag_word);
    // Read rest of board data.
    in_data.read(reinterpret_cast<char *>(&*frag.dataBegin() +
                                          header_size_bytes),
                 (board.event_size() - Board::header_size_words()) *
                 sizeof(Board::data_t));
    read_bytes += (frag.dataEnd() - frag.dataBegin()) *
                  sizeof(RawDataType);
    // Update fragment header.
    frag.updateSize();
    frag.setFragmentID(board.board_id());
    frag.setEventID(board.event_counter());
    if (in_data.eof()) {
      // Look for next file.
      in_data.close();
      ++next_point_.first;
      next_point_.second = 0;
    }
  }
  // Update counter for next time.
  if (in_data.is_open()) {
    next_point_.second = in_data.tellg();
  }
  return true;
}
