# project name
project(artdaq_Version)

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/GetReleaseVersion.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/GetReleaseVersion.cc @ONLY )

execute_process( COMMAND touch -c ${CMAKE_CURRENT_BINARY_DIR}/GetReleaseVersion.cc )

art_make_library( LIBRARY_NAME artdaq_Version
                  SOURCE ${CMAKE_CURRENT_BINARY_DIR}/GetReleaseVersion.cc
                 )

install_headers()
install_source()
