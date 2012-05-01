
simple_plugin(DS50CompressionChecker "module" artdaq_DAQdata NOINSTALL USE_BOOST_UNIT)

if(MPI_FOUND)
  cet_test(builder_art_compressOnly_t
    HANDBUILT
    TEST_EXEC builder.sh
    TEST_ARGS hosts-5-5-5.txt builder_art_compressOnly_t.fcl
    DATAFILES hosts-5-5-5.txt
    ../../artdaq/Compression/DS50Run002211.txt
    ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
    fcl/builder_art_compressOnly_t.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(builder_art_compressOnly_10_t
    HANDBUILT
    TEST_EXEC builder.sh
    TEST_ARGS hosts-5-5-10.txt builder_art_compressOnly_t.fcl
    DATAFILES hosts-5-5-10.txt
    ../../artdaq/Compression/DS50Run002211.txt
    ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
    fcl/builder_art_compressOnly_t.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(builder_art_compressOnly_15_t
    HANDBUILT
    TEST_EXEC builder.sh
    TEST_ARGS hosts-5-5-15.txt builder_art_compressOnly_t.fcl
    DATAFILES hosts-5-5-15.txt
    ../../artdaq/Compression/DS50Run002211.txt
    ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
    fcl/builder_art_compressOnly_t.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(builder_art_compressOnly_20_t
    HANDBUILT
    TEST_EXEC builder.sh
    TEST_ARGS hosts-5-5-20.txt builder_art_compressOnly_t.fcl
    DATAFILES hosts-5-5-20.txt
    ../../artdaq/Compression/DS50Run002211.txt
    ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
    fcl/builder_art_compressOnly_t.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(builder_art_compressDecompress_t
    HANDBUILT
    TEST_EXEC builder.sh
    TEST_ARGS hosts-5-5-5.txt builder_art_compressDecompress_t.fcl
    DATAFILES hosts-5-5-5.txt
    ../../artdaq/Compression/DS50Run002211.txt
    ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
    fcl/builder_art_compressOnly_t.fcl
    fcl/builder_art_compressDecompress_t.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(builder_art_compressDecompressCheck_t
    HANDBUILT
    TEST_EXEC builder.sh
    TEST_ARGS hosts-5-5-5.txt builder_art_compressDecompressCheck_t.fcl
    DATAFILES hosts-5-5-5.txt
    ../../artdaq/Compression/DS50Run002211.txt
    ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
    fcl/builder_art_compressOnly_t.fcl
    fcl/builder_art_compressDecompress_t.fcl
    fcl/builder_art_compressDecompressCheck_t.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )
endif()
