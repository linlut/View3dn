/*
File:   toolbox.h
Author: Nan Zhang
Date:   Jan. 30, 2002

*/
#ifndef _INC_ZNTOOLBOX_H__
#define _INC_ZNTOOLBOX_H__

//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
//#include <fcntl.h>
#include <math.h>
#include <time.h>


inline double GetClockInSecond(void)
{
	double t;

	t = (double)clock();
	t /=CLOCKS_PER_SEC ;
	return t;
}


inline void delayTimeMSecond(const double ms)
{
	const double K=1000;
	const double t0 = GetClockInSecond()*K;
	
	while (1){
		const double t = GetClockInSecond()*K;
		if (t-t0 > ms)
			break;
	}
}


inline void delayTimeSecond(const double sec)
{
	delayTimeMSecond(sec*1000);
}



#define POW(c) (1<<(c))
#define MASK(c) (((unsigned long)-1) / (POW(POW(c)) + 1))
#define ROUND(n, c) (((n) & MASK(c)) + ((n) >> POW(c) & MASK(c)))
// fast count bits of an integer
inline int bit_count(const unsigned int nn)
{
    unsigned int n = nn;
	n = ROUND(n, 0);
    n = ROUND(n, 1);
    n = ROUND(n, 2);
    n = ROUND(n, 3);
    n = ROUND(n, 4);
    return (int)n;
}

// another algorithm for fast counting
inline int bit_count2(unsigned int n)
{
    int count;
    for(count = 0; n; n &= n - 1){
        count++;
    }
    return count;
}


#endif


