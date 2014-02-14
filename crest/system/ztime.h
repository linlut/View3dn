//FILE: ztime.h
// This file includes functions for fast and accurate system clock access
// Nan Zhang   Oct. 5, 2007


#ifndef ZN_SYSTEM_ZTIME_H
#define ZN_SYSTEM_ZTIME_H

#include <time.h>

#ifdef WIN32
# include <windows.h>
#endif

#ifdef WIN32
typedef ULONGLONG ctime_t;
#else
typedef unsigned long long ctime_t;
#endif

class CTime
{
private:

public:
	// Get current reference time
	static volatile ctime_t getRefTime();

	// Get the frequency of the reference timer
	static ctime_t getRefTicksPerSec();

	// Get current time using the fastest available method
	static volatile ctime_t getFastTime();

	// Get the frequency of the fast timer
	static ctime_t getTicksPerSec();

	// Same as getFastTime, but with the additionnal guaranty that it will never decrease.
	static volatile ctime_t getTime();

	// Sleep for the given duration in second
	static void sleep(double s);

protected:
	static ctime_t computeTicksPerSec();
};


//given starting clock and ending clock, return the time difference in ms
double getTimeDiffMS(const ctime_t& t1, const ctime_t& t2);

void startFastTimer(const int id);

void stopFastTimer(const int id);

double reportTimeDifference(const int id, const char *str=NULL);

extern "C"
{
  void _cstartFastTimer(const int id);

  void _cstopFastTimer(const int id);

  double _creportTimeDifference(const int id, const char *str=NULL);
}


#endif


