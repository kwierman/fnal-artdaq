include(CetTest)
include(CheckUpsVersion)
 _check_if_version_greater(cetbuildtools ${CETBUILDTOOLS_VERSION} v2_01_05)

macro( install_fcl_files )
  cet_parse_args( IS "LIST" "" ${ARGN})
  set(fcl_install_dir ${product}/${version}/fcl )
  message( STATUS "install_fcl_files: FHICL files will be installed in ${fcl_install_dir}" )
  if( IS_LIST )
     INSTALL ( PROGRAMS  ${IS_LIST}
               DESTINATION ${fcl_install_dir} )
  else()
     message( FATAL_ERROR 
              "ERROR: call install_fcl_files with LIST only")
  endif()
endmacro( install_fcl_files )

if(WANT_MPI)
  include_directories(${MPI_INCLUDE_PATH})
  cet_test(daqrate PREBUILT NO_AUTO INSTALL_BIN)
  cet_test(builder.sh PREBUILT NO_AUTO INSTALL_BIN)
endif()

cet_test(numawrap PREBUILT NO_AUTO INSTALL_BIN)

# ----------------------------------------------------------------------
# Build this project's executables:

# Nominally a test exec (no MPI) but we want this to be available to other packages.
add_executable(driver driver.cc)
target_link_libraries(driver
  artdaq_DAQrate
  artdaq_DAQdata
  artdaq_Utilities
  ${ART_FRAMEWORK_ART}
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
install(TARGETS driver DESTINATION  ${flavorqual_dir}/bin)

if(WANT_MPI)
  add_executable( perfdump perfdump.cc )
  target_link_libraries(perfdump artdaq_DAQrate)

  add_executable( builder builder.cc )
  target_link_libraries(builder
    ${ART_FRAMEWORK_ART}
    artdaq_DAQrate
    artdaq_DAQdata
    artdaq_Utilities
    ${FHICLCPP}
    ${CETLIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${MPI_LIBRARIES}
    )
  install(TARGETS perfdump builder DESTINATION ${flavorqual_dir}/bin)

  install_fcl_files(LIST daqrate_simdata_noart.fcl)
endif()
