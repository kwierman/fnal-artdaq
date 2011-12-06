
#include "Mpi.hh"

struct Clocker
{
  Clocker(double& out): out_(out) { out = MPI_Wtime(); }
  ~Clocker() { out_ = MPI_Wtime() - out_; }

  double& out_;
};

MPIProg::MPIProg(int& argc, char**& argv)
{
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&procs_);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank_);
}

MPIProg::~MPIProg()
{
  MPI_Finalize();
}

