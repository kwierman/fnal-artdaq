# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

if(WANT_MPI)
  include_directories(${MPI_INCLUDE_PATH})
endif()

set( LIB_NAME artdaq_DAQrate )

set(LIB_SOURCES
  EventStore.cc
  GlobalQueue.cc
  MonitoredQuantity.cc
  SimpleQueueReader.cc
  StatisticsCollection.cc
  Utils.cc
  )

set(LIB_LIBS
  artdaq_DAQdata
  ${FHICLCPP}
  ${CLHEP}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

if(WANT_MPI)
  list(APPEND LIB_SOURCES
    Config.cc
    MPIProg.cc
    Perf.cc
    RHandles.cc
    SHandles.cc
    infoFilename.cc
    )
  list(APPEND LIB_LIBS ${MPI_LIBRARIES})
else()
  set_source_files_properties(EventStore.cc
    PROPERTIES COMPILE_DEFINITIONS ARTDAQ_NO_PERF
    )
endif()

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES})

target_link_libraries(artdaq_DAQrate ${LIB_LIBS})

# ----------------------------------------------------------------------
# Install this project's headers, sources, library, and executables:

install_headers()
install_source()
install( TARGETS ${LIB_NAME}
         DESTINATION ${flavorqual_dir}/lib )

#
# ======================================================================
