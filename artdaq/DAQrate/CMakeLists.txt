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
    detail/FragCounter.cc
    infoFilename.cc
    )
  list(APPEND LIB_LIBS ${MPI_C_LIBRARIES})
else()
  set_source_files_properties(EventStore.cc
    PROPERTIES COMPILE_DEFINITIONS ARTDAQ_NO_PERF
    )
endif()

art_make_library(SOURCE
  ${LIB_SOURCES}
  LIBRARIES
  ${LIB_LIBS}
)

install_headers(SUBDIRS detail)
install_source()
