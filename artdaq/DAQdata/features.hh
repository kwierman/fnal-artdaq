#ifndef artdaq_DAQdata_features_hh
#define artdaq_DAQdata_features_hh

// This header defines the CPP symbol USE_MODERN_FEATURES. This symbol
// can be used to hide code from Root dictionaries, as shown below
//
// Usage:
//
//   #if USE_MODERN_FEATURES
//      void function_that_you_want_to_hide() { ... };
//      void another_function_to_hide() {...};
//   #endif
//

#undef USE_MODERN_FEATURES
#if !defined(__GCCXML__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#define USE_MODERN_FEATURES 1
#endif

#endif /* artdaq_DAQdata_features_hh */
