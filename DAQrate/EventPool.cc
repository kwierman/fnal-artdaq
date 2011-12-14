
#include "EventPool.hh"
#include "Utils.hh"

#include <algorithm>

FragmentPool::FragmentPool(Config const& conf):
  seq_(),
  word_count_(conf.fragment_words_),
  rank_(conf.rank_),
  data_length_(word_count_*2),
  d_(data_length_),
  range_(data_length_ - word_count_)
{
  generate(d_.begin(),d_.end(),LongMaker());
}

FragmentPool::~FragmentPool()
{
}

void FragmentPool::operator()(Data& output)
{
  char* kab = getenv("HOSTNAME");
  std::string jab;
  if (kab != 0) {
      jab = kab;
  }
  else {
      jab = "unknown";
  }
  if(output.size()<word_count_) output.resize(word_count_);
  std::cout << "Inside FragmentPool with rank " << rank_
            << ", word_count_ = " << word_count_
            << ", host = " << jab << std::endl;

  int start = (LongMaker::make()) % range_;
  std::copy(d_.begin()+start, d_.begin()+start+word_count_, output.begin());

  FragHeader* h = (FragHeader*)&output[0];

  h->id_=seq_++;
  h->from_=rank_;
  h->time_ms_=1;
}
