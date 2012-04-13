include(CetTest)

cet_test(builder_art_compress_t HANDBUILT NO_AUTO
  TEST_EXEC mpirun_rsh
  TEST_ARGS --rsh -np 15 -hostfile hosts-5-5-5.txt
  -tv
  PRINT_HOST=1 ART_OPENMP_DIR=$ART_OPENMP_DIR
  FHICL_FILE_PATH="$FHICL_FILE_PATH"
  ${EXECUTABLE_OUTPUT_PATH}/builder 5 5 1 1
  -- -c ./builder_art_compress_t.fcl
  DATAFILES hosts-5-5-5.txt
  ../../artdaq/Compression/DS50Run002211.txt
  ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
  builder_art_compress_t.fcl
)
