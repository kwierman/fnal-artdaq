
#include <iostream>

using namespace std;

__global__ void add(int* a, int* b, int* c)
{
	*c = *a + *b;
}

ostream& operator<<(ostream& ost, cudaDeviceProp const& p)
{
  ost << "major=" << p.major << "\n";
  ost << "minor=" << p.minor << "\n";
  ost << "MP count=" << p.multiProcessorCount << "\n";
  ost << "name=" << p.name << "\n";
  ost << "clock rate=" << p.clockRate << "\n";
  ost << "deviceOverlap=" << p.deviceOverlap << "\n";
  ost << "maxThreadsPerBlock=" << p.maxThreadsPerBlock << "\n";
  ost << "maxThreadsPerMP=" << p.maxThreadsPerMultiProcessor << "\n";
  ost << "warpSize=" << p.warpSize << "\n";
  // ost << "asyncEngineCount=" << p.asyncEngineCount << "\n";
  // ost << "concurrentKernels=" << p.concurrentKernels << "\n";
  ost << "integrated=" << p.integrated << "\n";
  return ost;
}

int main(void) 
{
  int a=1,b=2,c=0;
  int *pa=0,*pb=0,*pc=0;

  cudaMalloc((void**)&pa,sizeof(int));
  cudaMalloc((void**)&pb,sizeof(int));
  cudaMalloc((void**)&pc,sizeof(int));

  cudaMemcpy(pa,&a,sizeof(int),cudaMemcpyHostToDevice);
  cudaMemcpy(pb,&b,sizeof(int),cudaMemcpyHostToDevice);

  add<<<1,1>>>(pa,pb,pc);

  cudaMemcpy(&c,pc,sizeof(int),cudaMemcpyDeviceToHost);

  cudaFree(pa);
  cudaFree(pb);
  cudaFree(pc);

  int dcount;
  cudaGetDeviceCount(&dcount);

  cudaDeviceProp prop0,prop1;
  cudaGetDeviceProperties(&prop0,0);
  cudaGetDeviceProperties(&prop1,0);

  cout << "c=" << c << endl;
  cout << "count=" << dcount << endl;
  cout << prop0 << "\n\n";
  cout << prop1 << "\n\n";

  return 0;
}
