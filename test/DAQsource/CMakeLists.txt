include(CetTest)

cet_test(driver_t HANDBUILT
         TEST_EXEC driver
         TEST_ARGS -c driver_t.fcl
         DATAFILES driver_t.fcl
         )

cet_test(read_files_t HANDBUILT
         TEST_EXEC art
         TEST_ARGS -c read_files_t.fcl
         DATAFILES read_files_t.fcl
        )