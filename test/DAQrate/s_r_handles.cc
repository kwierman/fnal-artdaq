#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/RHandles.hh"
#include "artdaq/DAQrate/SHandles.hh"

#include "artdaq/DAQdata/Debug.hh"

#include <iostream>
#include <mpi.h>
#include <stdlib.h> // for putenv

void do_sending(int /*my_rank*/, int num_senders, int num_receivers)
{
  artdaq::SHandles sender(10, // buffer_count
                          1024 * 1024, // max_payload_size
                          num_receivers, // dest_count
                          num_senders); // dest_start
}
void do_receiving(int /*my_rank*/, int num_senders)
{
  artdaq::RHandles receiver(10, // buffer_count
                            1024 * 1024, // max_payload_size
                            num_senders,
                            0);
  receiver.waitAll();
}

int main(int argc, char * argv[])
{
  char envvar[] = "MV2_ENABLE_AFFINITY=0";
  assert(putenv(envvar) == 0);
  auto const requested_threading = MPI_THREAD_SERIALIZED;
  int  provided_threading = -1;
  auto rc = MPI_Init_thread(&argc, &argv, requested_threading, &provided_threading);
  assert(rc == 0);
  assert(requested_threading == provided_threading);
  int my_rank = -1;
  rc = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  assert(rc == 0);
  if (my_rank == 0) {
    std::cout << "argc:" << argc << std::endl;
    for (int i = 0; i < argc; ++i) {
      std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
    }
  }
  if (argc != 2) {
    std::cerr << argv[0] << " requires 2 arguments, " << argc << " provided\n";
    return 1;
  }
  auto num_sending_ranks = atoi(argv[1]);
  int total_ranks = -1;
  rc = MPI_Comm_size(MPI_COMM_WORLD, &total_ranks);
  auto num_receiving_ranks = total_ranks - num_sending_ranks;
  if (my_rank == 0) {
    std::cout << "Total number of ranks:     " << total_ranks << std::endl;
    std::cout << "Number of sending ranks:   " << num_sending_ranks << std::endl;
    std::cout << "Number of receiving ranks: " << num_receiving_ranks << std::endl;
  }
  configureDebugStream(my_rank, 0);
  if (my_rank < num_sending_ranks)
  { do_sending(my_rank, num_sending_ranks, num_receiving_ranks); }
  else
  { do_receiving(my_rank, num_sending_ranks); }
  rc = MPI_Finalize();
  assert(rc == 0);
  return 0;
}
