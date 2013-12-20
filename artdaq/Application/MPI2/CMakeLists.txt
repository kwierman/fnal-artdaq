include_directories(${MPI_INCLUDE_PATH})

art_make_library(SOURCE
  BoardReaderApp.cc
  EventBuilderCore.cc
  EventBuilderApp.cc
  BoardReaderCore.cc
  AggregatorApp.cc
  AggregatorCore.cc
  StatisticsHelper.cc
  MPISentry.cc
  LIBRARIES
  artdaq_DAQrate
  artdaq_Application
  ${ART_FRAMEWORK_ART}
  ${MPI_C_LIBRARIES}
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${XMLRPC_CLIENTPP}
  ${XMLRPCPP}
)

art_make_exec(EventBuilderMain
  LIBRARIES
  artdaq_Application_MPI2
  artdaq_ExternalComms
)

art_make_exec(BoardReaderMain
  LIBRARIES
  artdaq_Application_MPI2
  artdaq_ExternalComms
)

art_make_exec(AggregatorMain
  LIBRARIES
  artdaq_Application_MPI2
  artdaq_ExternalComms
)


cet_script(pmt.rb pmt_cleanup.sh pmt_mpiwrapper.sh)# REMOVE_EXTENSIONS)

install_headers()
install_source()
