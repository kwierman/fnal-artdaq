include(CetTest)

cet_test(read_files_t HANDBUILT
         TEST_EXEC art
         TEST_ARGS -c read_files_t.fcl
         DATAFILES read_files_t.fcl
        )