include(CetTest)

link_libraries(artdaq)
cet_test(compress_t DATAFILES thirty-two-12-bit-table.txt)

