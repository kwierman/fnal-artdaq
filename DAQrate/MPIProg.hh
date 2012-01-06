#ifndef artdaq_DAQrate_MPIProg_HH
#define artdaq_DAQrate_MPIProg_HH


struct MPIProg
{
  MPIProg(int& argc, char**& argv);
  ~MPIProg();

  int procs_;
  int rank_;
};


#endif
