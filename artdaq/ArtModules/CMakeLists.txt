project(artdaq_bladder)

if(MPI_FOUND) # Have MPI
  # Remove cxx library -- compiled with wrong version of GCC.
  include_directories(${MPI_INCLUDE_PATH})
endif()

set(MAYBE_MPI_LIBS)
if(MPI_FOUND)
  list(APPEND MAYBE_MPI_LIBS ${MPI_LIBRARIES})
endif()

simple_plugin(DS50Compression "module" artdaq_Compression artdaq_DAQdata ${MAYBE_MPI_LIBS})
simple_plugin(DS50DataCharacterizer "module" artdaq_DAQdata)

find_library( CETLIB NAMES cetlib PATHS $ENV{CETLIB_LIB} )
find_library( FHICLCPP NAMES fhicl_cpp PATHS $ENV{FHICLCPP_LIB} )
