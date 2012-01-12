#ifndef UTILS_HHH
#define UTILS_HHH

#include <stdlib.h>
#include <sys/time.h>
#include <ostream>

struct LongMaker {
  long  operator()() const { return make(); }
  static long make() { return (long) lrand48(); }
};

inline double asDouble(struct timeval & t)
{
  return (double)t.tv_sec + (double)t.tv_usec / 1e6;
}

struct TimedLoop {
  TimedLoop(unsigned int seconds): _seconds(seconds), _count(), _start() {
    struct timeval t;
    gettimeofday(&t, 0);
    _start = t.tv_sec;
  }

  bool isDone() {
    bool rc = false;
    ++_count;
    if (_count % 20 == 0) {
      struct timeval t;
      gettimeofday(&t, 0);
      if ((t.tv_sec - _start) > _seconds) { rc = true; }
    }
    return rc;
  }

  unsigned int _seconds, _count;
  unsigned long _start;
};

std::ostream & getStream(int rank);
void writeData(int rank, const char* d, int size);

#endif
