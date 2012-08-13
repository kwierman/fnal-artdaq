if(WANT_MPI)
  include_directories(${MPI_INCLUDE_PATH})
  simple_plugin(DS50Compression "module" artdaq_Compression artdaq_DAQdata ${MPI_LIBRARIES})
endif()

simple_plugin(DS50DataCharacterizer "module" artdaq_DAQdata)
