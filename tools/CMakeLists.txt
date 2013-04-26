include_directories(${MPI_INCLUDE_PATH})

art_make_exec(StateResponder
  LIBRARIES
  artdaq_ExternalComms
  artdaq_Application
  ${MPI_LIBRARIES}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )

install_source()
