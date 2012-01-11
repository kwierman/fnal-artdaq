#include "DAQrate/Config.hh"

int main(int argc, char* argv[])
{
  int rank = 1;
  int nprocs = 5;
  Config cfg(rank, nprocs, argc, argv);
}
