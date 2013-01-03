#ifndef artdaq_Utilities_SimpleLookupPolicy_h
#define artdaq_Utilities_SimpleLookupPolicy_h

#include "cetlib/filepath_maker.h"
#include "cetlib/search_path.h"
#include <memory>

namespace artdaq {
  class SimpleLookupPolicy;
}

class artdaq::SimpleLookupPolicy : public cet::filepath_maker
{
public:

  enum ArgType : int { ENV_VAR = 0, PATH_STRING = 1 };

  SimpleLookupPolicy(std::string const &paths, ArgType argType = ENV_VAR);
  virtual std::string operator() (std::string const &filename);
  virtual ~SimpleLookupPolicy() noexcept;

private:
  std::unique_ptr<cet::search_path> cwdPath_;
  std::unique_ptr<cet::search_path> fallbackPaths_;

};

#endif /* artdaq_Utilities_SimpleLookupPolicy_h */

// Local Variables:
// mode: c++
// End:
