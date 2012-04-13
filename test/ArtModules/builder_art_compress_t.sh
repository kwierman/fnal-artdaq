#!/bin/bash

mpirun_rsh --rsh -np 15 -hostfile hosts-5-5-5.txt \
"PRINT_HOST=1" \
"ART_OPENMP_DIR=$ART_OPENMP_DIR" \
"FHICL_FILE_PATH=\"$FHICL_FILE_PATH\"" \
"LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH\"" \
"/home/greenc/work/cet-is/global-build/artdaq/bin/builder" \
5 5 1 1 -- -c "./builder_art_compress_t.fcl"
