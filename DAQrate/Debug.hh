#ifndef Debug_HHH
#define Debug_HHH

#include <iostream>

class Junker
{
public:
	template <class T>
	Junker& operator<<(T const&) { return *this; } 
	template <class T>
	Junker& operator<<(T&) { return *this; }
};

extern Junker junker;

std::ostream& PgetDebugStream();
void PconfigureDebugStream(int rank, int run);

#ifdef DEBUGME
#define Debug getDebugStream()
inline std::ostream& getDebugStream() { return PgetDebugStream(); }
inline void configureDebugStream(int rank, int run) { PconfigureDebugStream(rank,run); }
#define flusher std::endl
#else
#define Debug junker
inline void configureDebugStream(int rank, int run) { }
#define flusher "\n"
#endif

#endif
