
#include "EventPool.hh"
#include "DAQdata/Fragment.hh"
#include "DAQdata/RawData.hh"
#include "Utils.hh"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace artdaq;

FragmentPool::FragmentPool(Config const& conf):
  seq_(),
  word_count_(conf.fragment_words_),
  rank_(conf.rank_),
  data_length_(word_count_*2),
  d_(data_length_),
  range_(data_length_ - word_count_)
{
  std::ostringstream name;
  name << "board" << conf.offset_ << ".out";
  ifs_.open( name.str().c_str(), std::ifstream::in );
  std::cout << "name is " << name.str() << '\n';
  std::cout << "ifs_.is_open() is " << ifs_.is_open() << '\n';
  generate(d_.begin(),d_.end(),LongMaker());
}

FragmentPool::~FragmentPool()
{
}

void FragmentPool::operator()(Data& output)
{
  if(output.size()<word_count_) output.resize(word_count_);

  int start = (LongMaker::make()) % range_;
  std::copy(d_.begin()+start, d_.begin()+start+word_count_, output.begin());
  RawFragmentHeader* h = (RawFragmentHeader*)&output[0];
#if 1
  if (ifs_.is_open())
  {
    if (ifs_.eof())
      ifs_.seekg(0, std::ios::beg);

    char *cp=((char*)&output[0])+sizeof(RawFragmentHeader);
    DarkSideHeaderOverlay *dshop = (DarkSideHeaderOverlay*)cp;
    std::cout << "output.size()="<<output.size()<<" bytes="<<output.size()*4<<'\n';
    ifs_.read( cp, sizeof(DarkSideHeaderOverlay) );


    unsigned size=dshop->event_size_;

    
    if (output.size() < (size+sizeof(RawFragmentHeader)/sizeof(RawDataType)))
    {
      output.resize(size+sizeof(RawFragmentHeader)/sizeof(RawDataType));
      cp=((char*)&output[0])+sizeof(RawFragmentHeader);
      h = (RawFragmentHeader*)&output[0];
    }
    size *= sizeof(RawDataType);
    printf("frag size=%d fragwords=0x%lx\n", size, (unsigned long)h->word_count_ );
    size -= sizeof(DarkSideHeaderOverlay);
    ifs_.read( cp+sizeof(DarkSideHeaderOverlay), size );
  }
#endif

  h->event_id_=seq_++;
  h->fragment_id_=rank_;
  //h->time_ms_=1;
  h->word_count_ = word_count_;
}
