# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

include_directories(${MPI_INCLUDE_PATH})

art_make(SUBDIRS detail
  LIB_LIBRARIES
  artdaq_DAQdata
  ${ARTDAQ-CORE_CORE}
  ${ARTDAQ-CORE_DATA}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CLHEP}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY} 
  ${MPI_C_LIBRARIES}
  ${TRACE}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
