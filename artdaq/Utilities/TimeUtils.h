#ifndef artdaq_Utilities_TimeUtils_h
#define artdaq_Utilities_TimeUtils_h

#include <sys/time.h>
#include <string>

namespace artdaq {
namespace TimeUtils {

  std::string convertUnixTimeToString(time_t inputUnixTime);
  std::string convertUnixTimeToString(struct timeval const& inputUnixTime);
  std::string convertUnixTimeToString(struct timespec const& inputUnixTime);

}
}

#endif /* artdaq_Utilities_TimeUtils_h */

// Local Variables:
// mode: c++
// End:
