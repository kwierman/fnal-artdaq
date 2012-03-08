
1/6/2011
The algorithm for calculating from/to using % is wrong in Handles.cc.
The dest function and ctor contain this information.

--------
To compile the rate program using openmpi, do the following:

source setup.sh
make

--------
To run the application on the ds cluster, do the following:

# allocate two nodes, get interactive prompt
qsub -q test_ds -l nodes=2 -I -A lqcdadmin
# at the interactive prompt, do
mpirun -np 2 -hostfile $PBS_NODEFILE ./rate 100000

The rate program takes the packet size you want sent on the network
as a parameter.  See the comments inside the source file for
further information.

----------

problem with 2 nodes and 10 buffers using openmpi.
seems to work with event size of 36000, and fail with size 37000.
the 36000 byte event means 12000 bytes per fragment.

----------

need to add final "Waitall" to be sure all sending and receiving
is complete. Need utility function for this.  It is currently
written in "Handles.cc" as cleanup.

tried this:
mpirun -hostfile /tmp/nfile -n 8 ./builder 2 40 3 3 2 100000 4 1
and it failed.

I see
id=31 out to dest 3 from src 0
id=16 into dest 3 from src 0

id=7 into dest 7 from 0 ???
id=6 into dest 6 from 0 ???
id=31 out to dest 4 from src 1
id=30 out to dest 5 from src 2

-------------

runs:
1017  mpirun -hostfile /tmp/nfile -n 8 ./builder 2 400 3 3 2 100000 3 1
1024  mpirun -hostfile /tmp/nfile -n 8 ./builder 2 4000 3 3 2 200000 9 2
1025  mpirun -hostfile /tmp/nfile -n 8 ./builder 2 4000 3 3 2 1000000 9 3
1026  mpirun -hostfile /tmp/nfile -n 8 ./builder 2 9000 3 3 2 1000000 9 4
1027  mpirun -hostfile /tmp/nfile -n 8 ./builder 2 9000 3 3 2 3000000 9 5
1028  mpirun -hostfile /tmp/nfile -n 8 ./builder 2 9000 3 3 2 9000 9 6

