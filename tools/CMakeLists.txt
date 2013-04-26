include_directories(${MPI_INCLUDE_PATH})

art_make_exec(StateResponder
  LIBRARIES
  artdaq_Application_MPI2
  artdaq_Application
  artdaq_ExternalComms
  ${MPI_LIBRARIES}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )

install_source()
