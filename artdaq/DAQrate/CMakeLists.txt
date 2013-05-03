# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

include_directories(${MPI_INCLUDE_PATH})

set(LIB_SOURCES
  EventStore.cc
  GlobalQueue.cc
  MonitoredQuantity.cc
  Perf.cc
  RHandles.cc
  SHandles.cc
  SimpleQueueReader.cc
  StatisticsCollection.cc
  Utils.cc
  detail/FragCounter.cc
  infoFilename.cc
  )

set(LIB_LIBS
  artdaq_DAQdata
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CLHEP}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY} 
  ${MPI_C_LIBRARIES}
  )

art_make_library(SOURCE
  ${LIB_SOURCES}
  LIBRARIES
  ${LIB_LIBS}
  )

install_headers(SUBDIRS detail)
install_source()
