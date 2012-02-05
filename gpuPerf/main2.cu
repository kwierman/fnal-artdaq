
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include "alloc.hh"

using namespace std;

namespace cu
{
  // ----- will be useful when boost or c++11 is available
  class Stream
  {
  public:
    Stream() { cudaStreamCreate(&s_); cerr<<"cuda strea made\n"; }
    ~Stream() { cudaStreamDestroy(s_); cerr <<"cuda stream dtor\n";}

    operator cudaStream_t&() { return s_; }

  private:
    cudaStream_t s_;

    // really do not want this thing copied
    Stream(Stream const& c)
    { cudaStreamCreate(&s_); cerr<<"cuda stream copy\n"; }
  };

  template<typename T>
  class DataDeviceAddr
  {
  public:
    explicit DataDeviceAddr(size_t num_elements):ptr_(0)
    {
      cudaError_t err = cudaMalloc(&ptr_,num_elements*sizeof(T));
      if(err!=cudaSuccess) throw runtime_error("cudaMalloc failed");
    }
    ~DataDeviceAddr() { cudaFree(ptr_); }

    operator T*() { return (T*)ptr_; }
  private:
    void* ptr_;
    // no copy allowed
    DataDeviceAddr(DataDeviceAddr const&) { }
  };

  typedef std::vector<unsigned long, cu::allocator_host<unsigned long> > DataHostVec;

  class StreamVec
  {
  public:
    typedef std::vector<cudaStream_t> Streams;

    explicit StreamVec(size_t count):strs_(count)
    {
      for(Streams::iterator i=strs_.begin(),e=strs_.end();
	  i!=e;++i)
	cudaStreamCreate(&(*i));
    }
    ~StreamVec()
    {
      for(Streams::iterator i=strs_.begin(),e=strs_.end();
	  i!=e;++i)
	cudaStreamDestroy((*i));
    }

    cudaStream_t& operator[](int i) { return strs_[i]; }
  private:
    Streams strs_;
  };
};

// no good...
// typedef std::vector<cu::Stream> StreamVec;

struct GetRand {
  unsigned long operator()() const
  { long v = lrand48(); return (unsigned long)v; }
};

const unsigned long byte_count = 1ul<<30;

__global__ void kern_add(unsigned long* a, int size)
{
  *a += 1;
}

int main(int argc, char* argv[])
{
  if(argc<2)
    {
      cerr << "usage: " << argv[0] 
	   << " num_streams"
	   << "\n";
      return -1;
    }
  
  cudaDeviceProp props; 
  cudaGetDeviceProperties(&props,0);
  size_t max_threads_block = props.maxThreadsPerBlock;
  size_t max_threads_mp = props.maxThreadsPerMultiProcessor;

  size_t num_streams = atoi(argv[1]);
  size_t num_iters = argc>2?atoi(argv[2]):10;
  size_t num_elements = byte_count / sizeof(cu::DataHostVec::value_type);
  size_t elements_per = num_elements / num_streams;
  size_t bytes_per = elements_per * sizeof(cu::DataHostVec::value_type);

  cout << "allocating space for " << num_elements << " elements..." << endl;
  cu::StreamVec streams(num_streams);
  cout << "streams complete..." << endl;
  cu::DataHostVec h_in(num_elements);
  cout << "host in complete..." << endl;
  cu::DataHostVec h_out(num_elements);
  cout << "host out complete..." << endl;
  cu::DataDeviceAddr<unsigned long> d_mem(num_elements);
  cout << "device complete..." << endl;

  // sharedMemBytes - Dynamic shared-memory size per thread block in bytes
  int shared_mem = 0;
  // grid and block dimension can be specified in dim3 or int.
  // dim3 grid_dim(x,y,z);
  // dim3 block_dim(x,y,z);
  int grid_dim = 1;
  int block_dim = 512;
  int count_per_thread = byte_count / (grid_dim * block_dim * num_streams);

  // fill in_data with values
  cout << "filling data values..." << endl;
  generate(h_in.begin(),h_in.end(),GetRand());

  cout << "processing..." << endl;
  for(int iter=0;iter<num_iters;++iter)
    {
      int si = iter % num_streams;
      int di = si * elements_per;

      cout << "iter " << iter << " si=" << si << " di=" << di << endl;

      cudaMemcpyAsync(d_mem+di, &h_in[di],
		      bytes_per, cudaMemcpyHostToDevice, streams[si]);

      cout << "copy to device done... (" << &d_mem[di] << ")" << endl;

      kern_add<<<grid_dim, block_dim, shared_mem, streams[si]>>>
	(&d_mem[di], count_per_thread);

      cout << "kernel exec done..." << endl;

      cudaMemcpyAsync(&h_out[di], &d_mem[di],
		      bytes_per, cudaMemcpyDeviceToHost, streams[si]);

      cout << "copy to host done... (" << &d_mem[di] << ")" << endl;
    }

  cudaDeviceSynchronize();
  cout << "ending" << endl;
  return 0;
}
