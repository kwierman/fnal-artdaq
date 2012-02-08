
#include "FragmentPool.hh"
#include "DAQdata/Fragment.hh"
#include "DAQdata/RawData.hh"
#include "MonitoredQuantity.hh"
#include "Utils.hh"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace artdaq;

FragmentPool::FragmentPool(Config const& conf):
  seq_(),
  word_count_(conf.fragment_words_),
  rank_(conf.rank_),
  data_length_(word_count_*2),
  frag_(data_length_),
  range_(data_length_ - word_count_),
  ifs_(),
  debugPrintLevel_(0),
  wordsReadFromFile_(0),
  fileBufferWordOffset_(0)
{
  char *envVarPtr = getenv("FRAGMENT_POOL_DEBUG");
  if (envVarPtr != 0) debugPrintLevel_ = atoi(envVarPtr);

  std::ostringstream name;
  name << conf.data_dir_ << "board" << conf.offset_ << ".out";
  ifs_.open( name.str().c_str(), std::ifstream::in );
  std::cout << "name is " << name.str() << '\n';
  std::cout << "ifs_.is_open() is " << ifs_.is_open() << '\n';

  // if we successfully opened the file, pre-load the fragments
  if (ifs_.is_open())
  {
    unsigned dsHeaderWords = 1 +
      ((sizeof(DarkSideHeaderOverlay)-1) / sizeof(RawDataType));

    // ensure that the data buffer is large enough to hold the header
    if (frag_.size() < dsHeaderWords)
    {
      frag_.resize(dsHeaderWords);
    }

    // read in the first header
    char *cp = (char*)&frag_[0];
    DarkSideHeaderOverlay *dshop = (DarkSideHeaderOverlay*)cp;
    ifs_.read( cp, sizeof(DarkSideHeaderOverlay) );

    // calculate the expected data size
    unsigned firstFragmentWords = dshop->event_size_;
    int numberOfFragments = conf.total_events_;
    if (numberOfFragments > 10000) numberOfFragments = 10000;
    unsigned long dataSetWords = firstFragmentWords * numberOfFragments;
    if (debugPrintLevel_ >= 2)
    {
      std::cout << "Words in first fragment=" << firstFragmentWords
                << ", data set words = " << dataSetWords
                << ", data set bytes = " << (dataSetWords*sizeof(RawDataType))
                << '\n';
    }

    // reset the file handle so that we're ready to start looping
    ifs_.seekg(0, std::ios::beg);

    // read in each fragment
    artdaq::MonitoredQuantity fileReadMonitor(1, 1);
    cp = (char*)&frag_[0];
    dshop = (DarkSideHeaderOverlay*)cp;
    unsigned actualEventCount = 0;
    for (int idx = 0; idx < numberOfFragments; ++idx)
    {
      if (ifs_.eof()) break;

      // resize the buffer, if needed (unlikely)
      if (frag_.size() < (wordsReadFromFile_ + dsHeaderWords))
      {
        frag_.resize(wordsReadFromFile_ + dsHeaderWords);
        cp = ((char*)&frag_[0]) + (wordsReadFromFile_*sizeof(RawDataType));
        dshop = (DarkSideHeaderOverlay*)cp;
      }

      // read in the header
      ifs_.read( cp, sizeof(DarkSideHeaderOverlay) );
      fileReadMonitor.addSample(sizeof(DarkSideHeaderOverlay));

      // determine the size of this fragment
      unsigned fragmentWords = dshop->event_size_;

      // resize the buffer, if needed
      if (frag_.size() < (wordsReadFromFile_ + fragmentWords))
      {
        frag_.resize(wordsReadFromFile_ + (10 * fragmentWords));
        cp = ((char*)&frag_[0]) + (wordsReadFromFile_*sizeof(RawDataType));
        dshop = (DarkSideHeaderOverlay*)cp;
      }

      // finish reading the fragment
      unsigned fragmentBytes = fragmentWords * sizeof(RawDataType);
      fragmentBytes -= sizeof(DarkSideHeaderOverlay);
      ifs_.read( cp+sizeof(DarkSideHeaderOverlay), fragmentBytes );
      ++actualEventCount;
      fileReadMonitor.addSample(fragmentBytes);

      // update pointers and sizes
      wordsReadFromFile_ += fragmentWords;
      cp = ((char*)&frag_[0]) + (wordsReadFromFile_*sizeof(RawDataType));
      dshop = (DarkSideHeaderOverlay*)cp;
    }

    if (debugPrintLevel_ >= 3) {
      std::cout << "  Read in " << actualEventCount
                << " fragments for board_id " << dshop->board_id_
                << ", actual data words = " << wordsReadFromFile_
                << std::endl;
    }

    if (debugPrintLevel_ >= 1) {
      fileReadMonitor.calculateStatistics();
      artdaq::MonitoredQuantity::Stats stats;
      fileReadMonitor.getStats(stats);
      std::cout << "File input for detector " << rank_
                << ": bytes read =" << std::setw(12)
                << static_cast<long>(stats.fullValueSum)
                << ", read rate = "
                << (stats.fullValueRate / 1024.0 / 1024.0)
                << " MB/sec" << std::endl;
    }
  }

  // otherwise generate random data to be used
  else {
    generate(frag_.begin(),frag_.end(),LongMaker());
  }
}

void FragmentPool::operator()(Fragment& output)
{
  unsigned outputWordCount;

  if (wordsReadFromFile_ > 0)
  {
    if (fileBufferWordOffset_ >= wordsReadFromFile_)
    {
      fileBufferWordOffset_ = 0;
    }

    char *cp = (char*)&frag_[fileBufferWordOffset_];
    DarkSideHeaderOverlay *dshop = (DarkSideHeaderOverlay*)cp;
    unsigned fragmentWords = dshop->event_size_;

    unsigned rfHeaderWords = 1 +
      ((sizeof(RawFragmentHeader)-1) / sizeof(RawDataType));
    if (output.size() < (fragmentWords+rfHeaderWords))
    {
      output.resize(fragmentWords+rfHeaderWords);
    }

    std::copy(frag_.begin()+fileBufferWordOffset_,
              frag_.begin()+fileBufferWordOffset_+fragmentWords,
              output.begin()+rfHeaderWords);
    fileBufferWordOffset_ += fragmentWords;
    outputWordCount = fragmentWords + rfHeaderWords;

    if (debugPrintLevel_ >= 4)
    {
      std::cout << "Copied " << fragmentWords
                << " words from the file data buffer to the send buffer."
                << std::endl
                << "  " << fileBufferWordOffset_
                << " words from the file data buffer have been sent so far."
                << std::endl;
    }
  }
  else
  {
    if(output.size()<word_count_) output.resize(word_count_);

    int start = (LongMaker::make()) % range_;
    std::copy(frag_.begin()+start, frag_.begin()+start+word_count_, output.begin());
    outputWordCount = word_count_;
  }

  RawFragmentHeader* h = output.fragmentHeader();
  h->event_id_=seq_++;
  h->fragment_id_=rank_;
  //h->time_ms_=1;
  h->word_count_ = outputWordCount;
}
