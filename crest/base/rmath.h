//FILE: rmath.h

#ifndef _ZRMATH_INC_H
#define _ZRMATH_INC_H

#include <math.h>
#include <crest/base/config_assert.h>


// Works on gcc 3.x and 4.x
template<class real>
inline int rfloor(real r)
{
    static const double FLOATTOINTCONST=(1.5*(1LL<<(52-16)));
    union { double d;
        int i;
    } temp;
    temp.d = r+FLOATTOINTCONST;
    return ((temp.i)>>16);
}

template<class real>
inline int rnear(real r)
{
    static const double FLOATTOINTCONST_0_5=(1.5*(1LL<<(52-16)))+0.5;
    union { double d;
        int i;
    } temp;
    temp.d = r+FLOATTOINTCONST_0_5;
    return ((temp.i)>>16);
}

inline int rceil(double r)
{
    return -rfloor(-r);
}

template<class real>
inline real rabs(real r)
{
    return (r>=0)?r:-r;
}

template<class real>
inline real rmin(real r, real s)
{
    return (r<=s)?r:s;
}

template<class real>
inline real rmax(real r, real s)
{
    return (r>s)?r:s;
}

template<class T>
inline T rlerp(const T& a, const T& b, float f)
{
    return a+(b-a)*f;
}

template<class T>
inline T rsqrt(const T& a)
{
	return (T)sqrtf((float)a);
}

inline double rsqrt(const double& a)
{
#if defined(__GNUC__)
    return sqrt(a);
#else
    return (double)sqrtl((long double)a);
#endif
}

inline long double rsqrt(const long double& a)
{
    return sqrtl(a);
}

template<class T>
inline const T SQR(const T& a)
{
    return a*a;
}

template<class T>
inline const T MAX(const T &a, const T &b)
{
    return b > a ? (b) : (a);
}

template<class T>
inline const T MIN(const T &a, const T &b)
{
    return b < a ? (b) : (a);
}

template<class T>
inline const T SIGN(const T &a, const T &b)
{
    return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);
}

template<class T>
inline void SWAP(T &a, T &b)
{
    T dum=a;
    a=b;
    b=dum;
}

inline void shft3(double &a, double &b, double &c, const double d)
{
    a=b;
    b=c;
    c=d;
}

#endif


