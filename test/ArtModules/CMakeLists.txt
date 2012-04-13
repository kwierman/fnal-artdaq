include(CetTest)

cet_test(builder_art_compress_t.sh PREBUILT
  DATAFILES hosts-5-5-5.txt
  ../../artdaq/Compression/DS50Run002211.txt
  ../../artdaq/Compression/DS50Run002211.txt_reversed.txt
  builder_art_compress_t.fcl
  )
