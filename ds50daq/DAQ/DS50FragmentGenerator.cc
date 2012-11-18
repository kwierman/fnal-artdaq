#include "DS50FragmentGenerator.hh"
#include <boost/lexical_cast.hpp>

#include "cetlib/exception.h"

ds50::DS50FragmentGenerator::DS50FragmentGenerator(): run_number_(-1) {
}

void ds50::DS50FragmentGenerator::start (int run) { 
  if (run < 0) throw cet::exception("DS50FragmentGenerator") << "negative run number";

  should_stop_ = false; // no lock required: thread not started yet
  run_number_ = run; 

  perfreset ();

  start_ (); 
}


void ds50::DS50FragmentGenerator::pause () { pause_ (); }

void ds50::DS50FragmentGenerator::resume () { resume_ (); } 

void ds50::DS50FragmentGenerator::stop () { 
  std::unique_lock<std::mutex> lk(mutex_);
  should_stop_ = true;
  stats_.should_stop = true;
  stop_ (); 

  stop_wait_.wait (lk, [&](){ return !should_stop_; });
}

std::string ds50::DS50FragmentGenerator::report () { 
  std::lock_guard<std::mutex> lk(mutex_);
  return boost::lexical_cast<std::string>(stats_.should_stop) + report_ ();
}

void ds50::DS50FragmentGenerator::perfreset () {
  std::lock_guard<std::mutex> lk(mutex_);
  stats_.should_stop = should_stop_;
  stats_.run_number = run_number_;
  stats_.call_count = 0;
  stats_.avg_frag_size = 0;
  perfreset_ ();
}

bool ds50::DS50FragmentGenerator::should_stop () { return should_stop_; } // no lock required: modified within a lock and used in getNext__ within a lock


bool ds50::DS50FragmentGenerator::getNext_ (artdaq::FragmentPtrs & output) { 
  if (run_number_ < 0) start (0);

  std::lock_guard<std::mutex> lk(mutex_);

  bool r = getNext__(output); 

  if (!r) {
    should_stop_ = false;
    stop_wait_.notify_all ();
  }

  stats_.avg_frag_size = (stats_.avg_frag_size * stats_.call_count + output.size ()) / (stats_.call_count + 1);
  stats_.call_count ++;
  stats_.should_stop = should_stop_;

  return r;
}

