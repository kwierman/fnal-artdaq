include(CetTest)

cet_test(config_t LIBRARIES artdaq_DAQrate artdaq_DAQdata)

cet_test(config_with_art_t LIBRARIES artdaq_DAQrate artdaq_DAQdata
  TEST_PROPERTIES ENVIRONMENT ARTDAQ_DAQRATE_USE_ART=1
  )

find_package(MPI REQUIRED)
 # Remove cxx library -- compiled with wrong version of GCC.
LIST(REMOVE_AT MPI_LIBRARIES 0)
include_directories(${MPI_INCLUDE_PATH})

cet_test(cpi_t SOURCES ${MPI_INCLUDE_PATH}/../share/mvapich2/examples_graphics/cpi.c LIBRARIES ${MPI_LIBRARIES})

cet_test(daqrate_fileread_nosink HANDBUILT
  TEST_EXEC daqrate
  DATAFILES fcl/daqrate_fileread.fcl
  fcl/daqrate_fileread_nosink.fcl
  TEST_ARGS 5 1 3 1 -- -c daqrate_fileread_nosink.fcl HANDBUILT
)

cet_test(daqrate_fileread HANDBUILT
  TEST_EXEC daqrate
  DATAFILES fcl/daqrate_fileread.fcl
  TEST_ARGS 5 1 3 1 -- -c daqrate_fileread.fcl HANDBUILT
)

#cet_test(daqrate_trivial HANDBUILT
#         TEST_EXEC daqrate
#         TEST_ARGS 1 1 100 6000140 2 1
#         )

#cet_test(daqrate_simple HANDBUILT
#         TEST_EXEC daqrate
#         TEST_ARGS 5 5 100 6000140 2 1
#         )
