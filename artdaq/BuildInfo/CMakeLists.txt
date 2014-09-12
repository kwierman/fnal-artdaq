# project name
project(artdaq_BuildInfo)

# the utcDateTime variable is used by the GetPackageInfo code
# to report the build datetime
execute_process( COMMAND "date" "-u" "+%d-%b-%Y %H:%M:%S %Z" OUTPUT_VARIABLE rawUTCDate )
string( STRIP ${rawUTCDate} utcDateTime )

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/GetPackageInfo.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/GetPackageInfo.cc @ONLY )

art_make_library( LIBRARY_NAME artdaq_BuildInfo
                  SOURCE
		  ${CMAKE_CURRENT_BINARY_DIR}/GetPackageInfo.cc
                 )

install_headers()
install_source()
