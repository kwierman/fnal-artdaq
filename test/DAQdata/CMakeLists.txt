
cet_test(Fragment_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata
  )

cet_test(RawEvent_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata
  )

cet_test(FragmentGenerator_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata
  )

cet_test(GenericFragmentSimulator_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata_GenericFragmentSimulator_generator
  )
