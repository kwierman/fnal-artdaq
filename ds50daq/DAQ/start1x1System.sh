#!/bin/bash

mpirun -n 1 /home/biery/scratch/ds50daqBuild/bin/boardreader -p 5440 : -n 1 /home/biery/scratch/ds50daqBuild/bin/eventbuilder -p 5450
