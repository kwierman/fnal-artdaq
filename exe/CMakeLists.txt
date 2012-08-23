include(CetTest)
if(WANT_MPI)
  cet_test(daqrate PREBUILT NO_AUTO)
  cet_test(builder.sh PREBUILT NO_AUTO)
endif()
cet_test(numawrap PREBUILT NO_AUTO)

# ----------------------------------------------------------------------
# Build this project's executables:


set ( common_exec_link_libraries
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
)


set ( art_exec_link_libraries
  ${ART_PERSISTENCY_COMMON_DICT}
  ${ART_FRAMEWORK_ART}
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
)

if(WANT_MPI)
  add_executable( perfdump perfdump.cc )
  target_link_libraries(perfdump artdaq_DAQrate)

  add_executable( builder builder.cc )
  target_link_libraries(builder
    artdaq_DAQrate
    )
endif()
