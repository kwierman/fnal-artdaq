
cet_test(FragmentGenerator_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata ${ARTDAQ-CORE_DATA}
  )

cet_test(GenericFragmentSimulator_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata_GenericFragmentSimulator_generator
  )
