include_directories(${MPI_INCLUDE_PATH})

art_make_library(SOURCE
  BoardReaderApp.cc
  EventBuilder.cc
  EventBuilderApp.cc
  FragmentReceiver.cc
  MPISentry.cc
  LIBRARIES
  artdaq_DAQrate
  artdaq_Application
  ${ART_FRAMEWORK_ART}
  ${MPI_LIBRARIES}
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
)

art_make_exec(eventbuilder
  LIBRARIES
  artdaq_Application_MPI2
  artdaq_ExternalComms
)

art_make_exec(boardreader
  LIBRARIES
  artdaq_Application_MPI2
  artdaq_ExternalComms
)

cet_script(pmt.rb)

install_headers()
install_source()
