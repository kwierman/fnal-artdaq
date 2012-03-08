# Generated automatically from Makefile.in by configure.
##### User configurable options #####
# This is an example Makefile.in (or Makefile configured with mpireconfig)
# for the programs cpi, pi3, and cpilog.  

# Oink
# BIN_MPI_LOC = $(OPENMPI_FQ_DIR)/bin/
# LIB_MPI_LOC = $(OPENMPI_FQ_DIR)/lib/

# MacOS
# BIN_MPI_LOC = /usr/local/bin/
# LIB_MPI_LOC = /usr/local/lib/

# LQCD ds1
# BIN_MPI_LOC = /usr/local/openmpi/bin/
# LIB_MPI_LOC = /usr/local/openmpi/lib/


CFLAGS = -O2 -g  -DDEBUGME=1

CC          = $(BIN_MPI_LOC)mpicc
CCC         = $(BIN_MPI_LOC)mpicxx
F77         = $(BIN_MPI_LOC)mpif77
CLINKER     = $(BIN_MPI_LOC)mpicc
CCLINKER    = $(BIN_MPI_LOC)mpicxx
FLINKER     = $(BIN_MPI_LOC)mpif77
F90         = $(BIN_MPI_LOC)mpif90
F90LINKER   = $(BIN_MPI_LOC)mpif90	  
MAKE        = make --no-print-directory
SHELL       = /bin/sh
#
ifeq (${MAC_OSX},yes) 
    MATH_LIB=
else
   
    MATH_LIB=-lm
endif

### End User configurable options ###

LIBS =
FLIBS =
EXECS = builder

default: builder

# 
# The cp for pi3f90 is needed because different Fortran 90 compilers
# accept *different* suffixes.
# pi3f90 also wants an MPI module.  If modules not supported, don't 
# try to build pi3f90
all: $(EXECS)
	@if [ "0" = 1 -a -s simpleio.c ] ; then $(MAKE) simpleio ; fi
	@if [ "1" = 1 ] ; then $(MAKE) hello++ ; fi
	@if [ "0" = 0 ] ; then  \
		$(MAKE) pi3 ; \
		$(MAKE) pi3p; \
	fi
	@if [ "1" = 0 ] ; then $(MAKE) cpilog ; fi
	@if [ -n "" ] ; then \
	    if [ ! -s pi3f90. ] ; then \
		cp pi3f90.f90 pi3f90. ; fi ; \
	    $(MAKE) pi3f90 ; fi
	@if [ "ignore" != "ignore" ] ; then \
	    MPICH_USE_SHLIB="yes" ; export MPICH_USE_SHLIB ; \
	    $(MAKE) clean ; $(MAKE) cpi  ; \
	    if [ "0" = 0 ] ; then $(MAKE) pi3 ; fi ; \
	    if [ -n "" ] ; then $(MAKE) pi3f90 ; fi ; \
	fi

builder: builder.o Debug.o Utils.o Config.o Mpi.o EventPool.o Perf.o RHandles.o SHandles.o EventStore.o
	$(CCLINKER) -o $@ $+ $(MATH_LIB)

cpi: cpi.o 
	$(CLINKER) -o cpi cpi.o $(MATH_LIB)
cpip: cpip.o 
	$(CLINKER) -o cpip cpip.o $(MATH_LIB)

cpilog: cpilog.o 
	$(CLINKER) -o cpilog cpilog.o -lmpe $(MATH_LIB)

#
# This uses MPICH's -mpilog option to the mpicc command to add the automatic
# logging interface.
cpiwithlog: cpi.o
	$(CLINKER) -o cpiwithlog cpi.o -mpilog $(MATH_LIB)

pi3: pi3.o 
	$(FLINKER) -o pi3 pi3.o $(FLIBS)
pi3p: pi3p.o 
	$(FLINKER) -o pi3p pi3p.o $(FLIBS)

# Some Fortran 90 compilers do not accept a .f90 suffix (xlf90 for one)
pi3f90.o: pi3f90.
	$(F90) $(F90FLAGS) -c pi3f90.
pi3f90: pi3f90.o 
	$(F90LINKER) -o pi3f90 pi3f90.o $(FLIBS)

#
# Here are the rules for the optional package examples (ROMIO/C++)
simpleio: simpleio.o
	$(CLINKER) -o simpleio simpleio.o 
hello++: hello++.o
	$(CCLINKER) -o hello++ hello++.o
hello++.o: hello++.cc
	$(CCC) -c hello++.cc

perfdump: perfdump.o Perf.o Config.o
	$(CCLINKER) -o $@ $+

#
# The Solaris C++ compiler creates a directory containing miscellaneous
# data.  The action is not documented in the Solaris CC man pages
# IRIX C++ creates ii_files
# pgCC creates .ii and .ti files
# We also remove any copy of pi3f90 that is created (e.g., pi3f90.f; pi3f90.f90
# is the master file)
clean:
	-rm -f *.o *~ PI* $(EXECS) pi3 simpleio hello++ pi3f90 cpilog
	-rm -rf SunWS_cache ii_files pi3f90.f pi3p cpip *.ti *.ii

.c.o:
	$(CC) $(CFLAGS) -c $<
.cc.o:
	$(CCC) $(CFLAGS) -c $<
.f.o:
	$(F77) $(FFLAGS) -c $<
.f90.o:
	$(F90) $(F90FLAGS) -c $<
.SUFFIXES: .f90
