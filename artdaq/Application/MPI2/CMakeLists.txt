include_directories(${MPI_INCLUDE_PATH})

art_make_library( LIBRARY_NAME artdaq_Application_MPI2
  SOURCE
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
  ${TRACE}
)

art_make_exec(EventBuilderMain
  LIBRARIES
  artdaq_Plugins
  artdaq_DAQrate
  artdaq_Application_MPI2
  artdaq_ExternalComms
  artdaq_BuildInfo
)

art_make_exec(BoardReaderMain
  LIBRARIES
  artdaq_Plugins
  artdaq_DAQrate
  artdaq_Application_MPI2
  artdaq_ExternalComms
  artdaq_BuildInfo
)

art_make_exec(AggregatorMain
  LIBRARIES
  artdaq_Plugins
  artdaq_DAQrate
  artdaq_Application_MPI2
  artdaq_ExternalComms
  artdaq_BuildInfo
)


cet_script(pmt.rb pmt_cleanup.sh pmt_mpiwrapper.sh)# REMOVE_EXTENSIONS)

install_headers()
install_source()
