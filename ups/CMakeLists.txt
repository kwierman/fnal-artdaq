
execute_process( COMMAND date
                 OUTPUT_VARIABLE datime
                 OUTPUT_STRIP_TRAILING_WHITESPACE )

# create package configuration and version files

process_ups_files()

configure_file ( ${artdaq_SOURCE_DIR}/ups/artdaq-config.cmake.in
                 ${artdaq_BINARY_DIR}/ups/artdaq-config.cmake @ONLY )
configure_file ( ${artdaq_SOURCE_DIR}/ups/artdaq-config-version.cmake.in
                 ${artdaq_BINARY_DIR}/ups/artdaq-config-version.cmake @ONLY )

install (FILES ${artdaq_BINARY_DIR}/ups/artdaq-config.cmake
               ${artdaq_BINARY_DIR}/ups/artdaq-config-version.cmake
	 DESTINATION ${flavorqual_dir}/lib/artdaq-${version} )

