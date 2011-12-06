
#include "Utils.hh"
#include "Config.hh"
#include "Mpi.hh"

#include "mpi.h"

#include <math.h>
#include <sys/resource.h>

#include <ctime>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

#define MY_TAG 102


// -------------------------------

int sender(int secs, int receiver, int total_size, int packet_size)
{
  int total_longs = total_size / sizeof(long);
  int num_longs = packet_size / sizeof(long);

  Longs data;
  generate_n(back_inserter(data),total_longs,LongMaker());

#if 0
  cout << "secs=" << secs << " rec=" << receiver
       << " total_size=" << total_size << " packet_size=" << packet_size
       << " total_longs=" << total_longs << " num_longs=" << num_longs
       << endl;
#endif

  TimedLoop tl(secs);
  bool done=false;
  double time_val;
  int count=0;

  {
    Clocker c(time_val);

    while(!done)
      {
	size_t start = LongMaker::make() % (total_longs - num_longs);
	long old_value = data[start];
	
	// first byte is done word: 0=done, 1=continue
	done = tl.isDone();
	data[start]= done?0:1;
	
#if 0
	MPI_Request req;
	MPI_Isend(&data[start],num_longs,
		  MPI_LONG,receiver,
		  MY_TAG,MPI_COMM_WORLD,&req);
	MPI_Status stat;
	MPI_Wait(&req,&stat);
#else
	MPI_Send(&data[start],num_longs,
		 MPI_LONG,receiver,
		 MY_TAG,MPI_COMM_WORLD);
#endif
	data[start]=old_value;
	++count;
      }
  }
	
  double gb = (double)count * ((double)packet_size / 1e9);
  cout << getpid() << " s " << packet_size << " " << time_val  << " " 
       << gb << " " << (gb/time_val) << endl;

  return 0;
}

int reader(int secs, int sender, int total_size, int packet_size)
{
  int total_longs = total_size / sizeof(long);
  int num_longs = packet_size / sizeof(long);

  const int total_bufs = 2;
  vector<Longs> data(total_bufs);

  for(int i=0;i<total_bufs;++i) data[i].resize(num_longs);
  // generate_n(back_inserter(data),total_longs,LongMaker());

#if 0
  cout << "secs=" << secs << " snd=" << sender
       << " total_size=" << total_size << " packet_size=" << packet_size
       << " total_longs=" << total_longs << " num_longs=" << num_longs
       << endl;
#endif

  double time_val;
  int count=0;

  {
    Clocker c(time_val);
    vector<MPI_Request> req(total_bufs);
    MPI_Status stat;
    int index;
    int rc=0;

    // initial post of all
    for(int i=0;i<total_bufs;++i)
      MPI_Irecv(&((data[i])[0]),num_longs, MPI_LONG, sender, 
		MY_TAG, MPI_COMM_WORLD,&req[i]);
    do
      {
#if 1
	// wait for any of the buffers to be ready
	MPI_Waitany(total_bufs,&req[0],&index,&stat);
	rc = (data[index])[0];
	// cout << "index=" << index << " rc=" << rc << endl;
	if(rc==0) break;

	// repost the buffer that was filled
	MPI_Irecv(&((data[index])[0]),num_longs, MPI_LONG, sender, 
		  MY_TAG, MPI_COMM_WORLD,&req[index]);
#else
	MPI_Status stat;
	MPI_Recv(&data[0],num_longs, MPI_LONG, sender, 
		 MY_TAG, MPI_COMM_WORLD,&stat);
#endif
	++count;
      }
    while(1);

    // clean up the remaining buffers
    for(int i=0;i<total_bufs;++i)
      {
	if(i!=index)
	  {
	    MPI_Cancel(&req[i]);
	    MPI_Wait(&req[i],&stat);
	  }
      }
  }

  double gb = (double)count * ((double)packet_size / 1e9);
  cout << getpid() << " r " << packet_size << " " << time_val  << " " 
       << gb << " " << (gb/time_val) << endl;

  return 0;
}

// ---------------

class Program : MPIProg
{
public:
  typedef vector<long> Data;
  typedef vector<Data> Buffers;

  Program(int argc, char* argv[]);
  ~Program();

  void go();
  void source();
  void sink();

private:
  Config conf_;
  Buffers bufs_;
};


Program::Program(int argc, char* argv[]):
  MPIProg(argc,argv),
  conf_(rank_,procs_,argc,argv),
  bufs_(conf_.buffer_count_)
{
}

Program::~Program()
{
}

void Program::go()
{
  
}

void Program::source()
{
  EventPool ep(conf_);

  // not using the run time method
  // TimedLoop tl(conf_.run_time_);

  for(int i=0;i<conf_.total_events_;++i)
    {
      
    }
  
}

void Program::sink()
{
}

// ---------------

int driver()
{
  unsigned int size = 1<<12;

  while(size<=(1<<23))
    {
      if(myid==source)
	rc=sender(run_time,sink,total_size,size);
      else
	rc=reader(run_time,source,total_size,size);

      MPI_Barrier(MPI_COMM_WORLD);

      size = size<<1;
    }
  return 0;
}

int main( int argc, char *argv[])
{
  int rc = -1;

  try
    {
      Program p(argc,argv);
      p.go();
      rc=0;
    }
  catch(string& s)
    {
      cerr << "yuck - " << s << "\n";
    }
  catch(const char* c)
    {
      cerr << "yuck - " << c << "\n";
    }

  return rc;
}


void printUsage()
{
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  cout << myid << ":"
       << " user=" << asDouble(usage.ru_utime) 
       << " sys=" << asDouble(usage.ru_stime)
       << endl;
}
            
