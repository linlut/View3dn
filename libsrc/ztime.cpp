//FILE: ztime.cpp

#include <ztime.h>

#ifdef WIN32
# include <windows.h>
# if _MSC_VER >= 1400
#  include <intrin.h>
# endif
#else
# include <unistd.h>
# include <sys/time.h>
#endif
#if defined(__ia64__) && (defined(__EDG_VERSION) || defined(__ECC))
#include <ia64intrin.h>
#endif
#include <iostream>
#include <stdio.h>
#include <stdlib.h>


#ifdef WIN32

volatile ctime_t CTime::getRefTime()
{
	LARGE_INTEGER a;
	QueryPerformanceCounter(&a);
	return(a.QuadPart);
}

ctime_t CTime::getRefTicksPerSec()
{
	LARGE_INTEGER b;
	QueryPerformanceFrequency(&b);
	return(b.QuadPart);
}

void CTime::sleep(double a)
{
	_sleep((long)(a*1000.0));
}

#else /* WIN32 */

volatile ctime_t CTime::getRefTime()
{
	struct timeval tv;
	gettimeofday(&tv,0);
	return ((ctime_t)tv.tv_sec)*1000000 + ((ctime_t)tv.tv_usec);
}

ctime_t CTime::getRefTicksPerSec()
{
	return (ctime_t)1000000;
}

void CTime::sleep(double a)
{
	usleep((unsigned int)(a*1000000.0));
}

#endif /* WIN32 */



#ifdef SOFA_RDTSC
#if defined(_MSC_VER)
volatile ctime_t CTime::getFastTime()
{
#if _MSC_VER >= 1400
	return __rdtsc();
#else
    _asm    _emit 0x0F
    _asm    _emit 0x31
#endif
}
#elif defined(__ia64__)
# if defined(__EDG_VERSION) || defined(__ECC)
volatile ctime_t CTime::getFastTime()
{
	return __getReg(_IA64_REG_AR_ITC);
}
# else
volatile ctime_t CTime::getFastTime()
{
	ctime_t t;
	__asm__ __volatile__("mov %0=ar.itc" : "=r"(t) :: "memory");
	return t;
}
# endif
#elif defined(__powerpc__) || defined(__POWERPC__)
volatile ctime_t CTime::getFastTime()
{
    register unsigned long t_u;
	register unsigned long t_l;
	asm volatile ("mftbu %0" : "=r" (t_u) );
	asm volatile ("mftb %0" : "=r" (t_l) );
	return (((ctime_t)t_u)<<32UL) | t_l;
}
#elif defined(__GNUC__)
volatile ctime_t CTime::getFastTime()
{
	ctime_t t;
	__asm__ volatile ("rdtsc" : "=A" (t) );
	return t;
}
#else
#error RDTSC not supported on this platform
#endif

ctime_t CTime::computeTicksPerSec()
{
	ctime_t tick1,tick2,tick3,tick4, time1,time2;
	ctime_t reffreq = getRefTicksPerSec();
	
	tick1 = getFastTime();
	time1 = getRefTime();
	tick2 = getFastTime();
	
	sleep(0.1);

	tick3 = getFastTime();
	time2 = getRefTime();
	tick4 = getFastTime();
	
	double cpu_frequency_min =(tick3-tick2) * ((double)reffreq) / (time2 - time1);
	double cpu_frequency_max =(tick4-tick1) * ((double)reffreq) / (time2 - time1);
	std::cout << " CPU Frequency in [" << ((int)(cpu_frequency_min/1000000))*0.001 << " .. " << ((ctime_t)(cpu_frequency_max/1000000))*0.001 << "] GHz" << std::endl;
	return (ctime_t)((cpu_frequency_min + cpu_frequency_max)/2);
}

ctime_t CTime::getTicksPerSec()
{
	static const ctime_t freq = computeTicksPerSec();
	return freq;
}

#else /*  */

volatile ctime_t CTime::getFastTime()
{
	return getRefTime();
}

ctime_t CTime::getTicksPerSec()
{
	return getRefTicksPerSec();
}

#endif /*  */

volatile ctime_t CTime::getTime()
{
	static ctime_t last = 0;
	ctime_t t = getFastTime();
	if (t <= last)
		t = last;
	else
		last = t;
	return t;
}

//return in mili-secons
double getTimeDiffMS(const ctime_t& t1, const ctime_t& t2)
{
	const ctime_t ticks = CTime::getTicksPerSec();
	const ctime_t dt = t2 - t1;
	ctime_t ms = dt*100000/ticks;
	long msi = ms;
	return ((double)msi)*0.01;
}


const int N=10;
static ctime_t tmstart[N], tmend[N];
void startFastTimer(const int i)
{
	if (i<0 || i>=N){
		printf("Error: timer ID exceed range!\n");
		return;
	}
	tmstart[i] = CTime::getTime();
}

void stopFastTimer(const int i)
{
	if (i<0 || i>=N){
		printf("Error: timer ID exceed range!\n");
		return;
	}
	tmend[i] = CTime::getTime();
}

double getTimeDifference(const int id)
{
	const double t = getTimeDiffMS(tmstart[id], tmend[id]);
	return t;
}

double reportTimeDifference(const int id, const char * str)
{
	if (id<0 || id>=N) return 0;
	char promptbuff[]="Time diff is ";
	char *p = &promptbuff[0];
	if (str) p = (char*)str;
	const double t = getTimeDifference(id);
	if (str)
		printf("%s : %lg ms\n", p, t);
	return t;
}
