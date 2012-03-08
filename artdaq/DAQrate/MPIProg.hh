#ifndef artdaq_DAQrate_MPIProg_hh
#define artdaq_DAQrate_MPIProg_hh


struct MPIProg
{
  MPIProg(int argc, char** argv);
  ~MPIProg();

  int procs_;
  int rank_;
};


#endif /* artdaq_DAQrate_MPIProg_hh */
