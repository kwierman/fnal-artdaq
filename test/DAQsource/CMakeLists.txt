
find_library( CETLIB NAMES cetlib PATHS $ENV{CETLIB_LIB} )
find_library( ART_PERSISTENCY_COMMON_DICT NAMES art_Persistency_Common_dict PATHS $ENV{ART_OPENMP_LIB} )
find_library( ART_FRAMEWORK_ART NAMES art_Framework_Art PATHS $ENV{ART_OPENMP_LIB} )
find_library( ART_FRAMEWORK_CORE NAMES art_Framework_Core PATHS $ENV{ART_OPENMP_LIB} )
find_library( ART_FRAMEWORK_SERVICES_REGISTRY NAMES art_Framework_Services_Registry PATHS $ENV{ART_OPENMP_LIB} )
find_library( ART_UTILITIES NAMES art_Utilities PATHS $ENV{ART_OPENMP_LIB} )
find_library( FHICLCPP NAMES fhicl_cpp PATHS $ENV{FHICLCPP_LIB} )
find_library( SIGC    NAMES sigc-2.0  PATHS $ENV{LIBSIGCPP_LIB} NO_DEFAULT_PATH )

simple_plugin(FragmentSniffer "module" artdaq_DAQdata)

cet_test(raw_event_queue_reader_t USE_BOOST_UNIT
         LIBRARIES artdaq_DAQsource artdaq_DAQdata_dict artdaq_DAQdata)

if(MPI_FOUND)
  cet_test(daq_flow_t
    LIBRARIES
    artdaq_DAQrate
    artdaq_DAQdata
    ${CETLIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${FHICLCPP}
    ${ART_PERSISTENCY_COMMON_DICT}
    artdaq_DAQrate
    ${ART_FRAMEWORK_ART}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    ${ART_UTILITIES}
    ${SIGC}
    ${Boost_FILESYSTEM_LIBRARY}
    ${ART_FRAMEWORK_CORE}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${ROOT_CINTEX}
    ${ROOT_PHYSICS}
    ${ROOTSYS}/lib/libGraf.so
    ${ROOT_TREE}
    ${ROOT_HIST}
    ${ROOT_MATRIX}
    ${ROOT_NET}
    ${ROOT_MATHCORE}
    ${ROOT_RIO}
    ${ROOT_THREAD}
    ${ROOT_CORE}
    ${ROOT_CINT}
    ${ROOT_REFLEX}
    ${CPPUNIT}
    -ldl
    TEST_ARGS -c daq_flow_t.fcl
    DATAFILES daq_flow_t.fcl
    )

  cet_test(driver_t HANDBUILT
    TEST_EXEC driver
    TEST_ARGS -c driver_t.fcl
    DATAFILES driver_t.fcl
    ${CMAKE_SOURCE_DIR}/artdaq/DAQdata/ds50_hist.dat
    )
endif()

cet_test(read_files_t HANDBUILT
         TEST_EXEC art
         TEST_ARGS -c read_files_t.fcl
         DATAFILES read_files_t.fcl
        )
