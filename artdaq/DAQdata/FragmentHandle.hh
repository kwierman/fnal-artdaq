#ifndef artdaq_DAQdata_FragmentHandle_hh
#define artdaq_DAQdata_FragmentHandle_hh

namespace artdaq {
  class Fragment;

  class FragmentHandle;
}

class artdaq::FragmentHandle {
public:
  FragmentHandle(Fragment &);
protected:
  Fragment & frag_;
};

artdaq::FragmentHandle::
FragmentHandle(Fragment & frag)
:
  frag_(frag)
{
}
#endif /* artdaq_DAQdata_FragmentHandle_hh */
