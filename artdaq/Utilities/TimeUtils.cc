#include "artdaq/Utilities/TimeUtils.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace BPT = boost::posix_time;

/**
 * Free function to format a UNIX time_t as a datetime string.
 */
std::string artdaq::TimeUtils::
convertUnixTimeToString(time_t inputUnixTime)
{
    // whole seconds
    BPT::ptime posixTime = BPT::from_time_t(inputUnixTime);
    std::string workingString = BPT::to_simple_string(posixTime);
    workingString.append(" UTC");

    return workingString;
}

/**
 * Free function to format a UNIX timeval as a datetime string.
 */
std::string artdaq::TimeUtils::
convertUnixTimeToString(struct timeval const& inputUnixTime)
{
    // deal with whole seconds first
    BPT::ptime posixTime = BPT::from_time_t(inputUnixTime.tv_sec);
    std::string workingString = BPT::to_simple_string(posixTime);

    // now fractional seconds
    char fractionalString[20];
    sprintf(fractionalString, "%06ld", inputUnixTime.tv_usec);
    workingString.append(".");
    workingString.append(fractionalString);
    workingString.append(" UTC");

    return workingString;
}

/**
 * Free function to format a UNIX timespec as a datetime string.
 */
std::string artdaq::TimeUtils::
convertUnixTimeToString(struct timespec const& inputUnixTime)
{
    // deal with whole seconds first
    BPT::ptime posixTime = BPT::from_time_t(inputUnixTime.tv_sec);
    std::string workingString = BPT::to_simple_string(posixTime);

    // now fractional seconds
    char fractionalString[20];
    sprintf(fractionalString, "%09ld", inputUnixTime.tv_nsec);
    workingString.append(".");
    workingString.append(fractionalString);
    workingString.append(" UTC");

    return workingString;
}
