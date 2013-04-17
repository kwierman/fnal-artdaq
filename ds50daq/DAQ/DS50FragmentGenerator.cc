#include "DS50FragmentGenerator.hh"
#include "fhiclcpp/ParameterSet.h"
#include <boost/lexical_cast.hpp>
#include "cetlib/exception.h"
#include <unistd.h>

ds50::DS50FragmentGenerator::DS50FragmentGenerator(const fhicl::ParameterSet &ps): run_number_(-1) {
  fragment_id_ = ps.get<int> ("fragment_id");
  sleep_us_ = ps.get<int> ("sleep_us", 0);
  init_only_ = ps.get<bool> ("init_only", false);
}

void ds50::DS50FragmentGenerator::start (int run) { 
  if (run < 0) throw cet::exception("DS50FragmentGenerator") << "negative run number";

    // thread not started yet
  ev_counter_.store (1);
  should_stop_.store (false);
  run_number_ = run; 

  perfreset ();

  start_ (); 
}


void ds50::DS50FragmentGenerator::pause () { 
  std::unique_lock<std::mutex> lk(mutex_);
  should_stop_ = true;
  pause_ (); 
}

void ds50::DS50FragmentGenerator::resume () { 
  should_stop_ = false; // no lock required: thread not started yet
  resume_ (); 
} 

void ds50::DS50FragmentGenerator::stop () { 
  should_stop_.store (true);
  std::unique_lock<std::mutex> lk(mutex_);
  stop_ (); 
}

std::string ds50::DS50FragmentGenerator::report () { 
  std::lock_guard<std::mutex> lk(mutex_);
  return boost::lexical_cast<std::string>(stats_.read_count) + report_ ();
}

void ds50::DS50FragmentGenerator::perfreset () {
  std::lock_guard<std::mutex> lk(mutex_);
  stats_.run_number = run_number_;
  stats_.read_count = stats_.call_count = 0;
  stats_.avg_frag_size = 0;
  perfreset_ ();
}

void ds50::DS50FragmentGenerator::shutdown () { 
  shutdown_ (); 
}
  
bool ds50::DS50FragmentGenerator::should_stop () { return should_stop_.load (); } 


bool ds50::DS50FragmentGenerator::getNext_ (artdaq::FragmentPtrs & output) { 
  if (init_only_) stop ();

  if (sleep_us_ > 0) usleep (sleep_us_);
  if (should_stop ()) usleep (500000);  // This will allow to the stopping thread to gather the required lock

  bool r = false;
  {
    std::lock_guard<std::mutex> lk(mutex_);

    stats_.read_count -= output.size ();

    r = getNext__(output); 

    stats_.avg_frag_size = (stats_.avg_frag_size * stats_.call_count + output.size ()) / (stats_.call_count + 1);
    stats_.call_count ++;
    stats_.read_count += output.size ();

  }

  return r;
}

