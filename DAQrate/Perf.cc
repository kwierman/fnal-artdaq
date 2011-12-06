
#include "Perf.hh"

#include "mpi.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

namespace {

  const char* id_names[] = { 
    "none","send","found","sent","recv",
    "woke","post","evtstart","evtend","jobstart","jobend"
  };

  struct Perf
  {
    typedef std::vector<unsigned char> Buf;

    Perf();
    ~Perf();

    void configure(Config const&);  
    static Perf* instance();

    template <class T> void write( T const& write_me);
  
    int pos_;
    int rank_;
    int run_;
    int type_;
    double start_;
    Buf data_;
    std::string filename_;
  };

  template <class T> void Perf::write(T const& w)
  {
    if (pos_+sizeof(T) > data_.size()) 
      {
	size_t cur=data_.size();
	data_.resize( cur + 50*1000*1000);
      }

    memcpy((void*)&data_[pos_],(const void*)&w,sizeof(T));
    pos_+=sizeof(T);
  }

  Perf* Perf::instance()
  {
    static Perf p;
    return &p;
  }

  Perf::Perf():pos_(),rank_(), run_(), type_(), start_(), filename_("NONE")
  { }

  Perf::~Perf()
  {
    // write out the samples
    ofstream ostr(filename_.c_str(),ofstream::binary);
    ostr.write((const char*)&data_[0],pos_);
  }

  void Perf::configure(Config const& conf)
  {
    int totalfrags = conf.totalReceiveFragments();
    int res_size = 0;

    rank_ = conf.rank_;
    run_ = conf.run_;
    type_ = conf.type_==Config::TaskSink?JobStartMeas::SINK:
      conf.type_==Config::TaskSource?JobStartMeas::SOURCE:JobStartMeas::DETECTOR;
    start_ = MPI_Wtime();
    filename_ = conf.infoFilename("perf_");
    
    if(type_!=JobStartMeas::SINK) res_size+=sizeof(SendMeas)*totalfrags;
    if(type_!=JobStartMeas::DETECTOR) res_size+=sizeof(RecvMeas)*totalfrags;
    res_size+=sizeof(EventMeas)*conf.total_events_;
    res_size+=1000*1000*100;

    data_.reserve(res_size);
  }

}

double PerfGetStartTime()
{ return Perf::instance()->start_; }

void PerfSetStartTime()
{ Perf::instance()->start_ = MPI_Wtime(); }

void PerfConfigure(Config const& conf)
{ Perf::instance()->configure(conf); }

void PerfWriteJobStart()
{ JobStartMeas js; }

void PerfWriteJobEnd()
{ JobEndMeas je; }

void PerfWriteEvent(EventMeas::Type t, int event_id)
{ EventMeas e(t,event_id); }

// ------------
CommonMeas::CommonMeas():buf_(),event_(),enter_(MPI_Wtime()),exit_() { }
CommonMeas::~CommonMeas() {}
void CommonMeas::complete() { exit_=MPI_Wtime(); }

// ------------
SendMeas::SendMeas():com_(),dest_(),found_() { }
SendMeas::~SendMeas() 
{
  com_.complete();
  Perf::instance()->write(*this);
}

void SendMeas::found(int event, int buf, short dest)
{
  com_.set(event,buf);
  dest_=dest;
  found_=MPI_Wtime();
}

// ------------
RecvMeas::RecvMeas():com_(),from_(),wake_() { }
RecvMeas::~RecvMeas() 
{
  com_.complete(); 
  Perf::instance()->write(*this);
}

void RecvMeas::woke(int event, int which_buf)
{
  com_.set(event,which_buf);
  wake_=MPI_Wtime();
}


// ------------
JobStartMeas::JobStartMeas():run_(Perf::instance()->run_),rank_(Perf::instance()->rank_),
			     which_((Type)Perf::instance()->type_),when_(MPI_Wtime()) { }
JobStartMeas::~JobStartMeas() 
{ Perf::instance()->write(*this); }

// ------------
JobEndMeas::JobEndMeas():when_(MPI_Wtime()) { }
JobEndMeas::~JobEndMeas() 
{ Perf::instance()->write(*this); }

// ------------
EventMeas::EventMeas(Type id, int event):which_(id),event_(event),when_(MPI_Wtime()) { }
EventMeas::~EventMeas() 
{ Perf::instance()->write(*this); }

// ---------------
const char* PerfGetName(int id) { return id<PERF_ID_END ? id_names[id] : "NA"; }
  

