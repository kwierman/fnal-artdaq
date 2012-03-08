#include "artdaq/Compression/BlockReader.hh"

#include <istream>

BlockReader::BlockReader(std::istream & ist): ist_(&ist), buffer_(chunk_size_counts)
{ }

reg_type BlockReader::next(ADCCountVec & out)
{
  // this entire implementation is not good
  if (buffer_.size() < chunk_size_counts) { buffer_.resize(chunk_size_counts); }
  size_t bytes_read = ist_->readsome((char*)&buffer_[0], chunk_size_bytes);
  reg_type rc = (ist_->eof() || bytes_read == 0) ? 0 : bytes_read / sizeof(ADCCountVec::value_type);
#if 0
  cout << "read bytes=" << bytes_read
       << " chunk_size_counts=" << chunk_size_counts
       << " rc=" << rc
       << "\n";
#endif
  if (rc) {
    out.swap(buffer_);
    out.resize(rc);
  }
  return rc;
}
