include(CetTest)

cet_test(config_t LIBRARIES artdaq_DAQrate artdaq_DAQdata)

cet_test(config_with_art_t LIBRARIES artdaq_DAQrate artdaq_DAQdata
  TEST_PROPERTIES ENVIRONMENT ARTDAQ_DAQRATE_USE_ART=1
  )

find_package(MPI REQUIRED)
 # Remove cxx library -- compiled with wrong version of GCC.
LIST(REMOVE_AT MPI_LIBRARIES 0)
include_directories(${MPI_INCLUDE_PATH})

cet_test(cpi_t SOURCES ${MPI_INCLUDE_PATH}/../share/examples_graphics/cpi.c LIBRARIES ${MPI_LIBRARIES})

cet_test(daqrate_fileread_nosink HANDBUILT
  TEST_EXEC daqrate
  DATAFILES fcl/daqrate_fileread.fcl
  fcl/daqrate_fileread_nosink.fcl
  TEST_ARGS 5 0 3 1 -- -c daqrate_fileread_nosink.fcl HANDBUILT
  TEST_PROPERTIES RUN_SERIAL true
)

cet_test(daqrate_fileread HANDBUILT
  TEST_EXEC daqrate
  DATAFILES fcl/daqrate_fileread.fcl
  TEST_ARGS 5 1 3 1 -- -c daqrate_fileread.fcl HANDBUILT
  TEST_PROPERTIES RUN_SERIAL true
)

cet_test(daqrate_perf_nosink_01_t
  HANDBUILT
  OPTIONAL_GROUPS PERF;LONG
  TEST_EXEC builder.sh
  TEST_ARGS -n -d 5 hosts-5-5-0.txt builder_perf_nosink.fcl
  DATAFILES hosts-5-5-0.txt
  fcl/builder_perf_nosink.fcl
  TEST_PROPERTIES RUN_SERIAL true
  )

cet_test(daqrate_perf_nosink_02_t
  HANDBUILT
  OPTIONAL_GROUPS PERF;LONG
  TEST_EXEC builder.sh
  TEST_ARGS -n -d 5 hosts-5-5-0-mix.txt builder_perf_nosink.fcl
  DATAFILES hosts-5-5-0-mix.txt
  fcl/builder_perf_nosink.fcl
  TEST_PROPERTIES RUN_SERIAL true
  )

cet_test(daqrate_perf_nosink_03_t
  HANDBUILT
  OPTIONAL_GROUPS PERF;LONG
  TEST_EXEC builder.sh
  TEST_ARGS -n -d 1 hosts-1-1-0.txt builder_perf_nosink.fcl
  DATAFILES hosts-1-1-0.txt
  fcl/builder_perf_nosink.fcl
  TEST_PROPERTIES RUN_SERIAL true
  )

cet_test(daqrate_perf_nosink_04_t
  HANDBUILT
  OPTIONAL_GROUPS PERF;LONG
  TEST_EXEC builder.sh
  TEST_ARGS -n -d 1 hosts-1-1-0-mix.txt builder_perf_nosink.fcl
  DATAFILES hosts-1-1-0-mix.txt
  fcl/builder_perf_nosink.fcl
  TEST_PROPERTIES RUN_SERIAL true
  )
