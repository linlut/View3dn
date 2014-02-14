//FILE: vector6d.h

#ifndef __ZVECTOR6d_2011__
#define __ZVECTOR6d_2011__

#include <math.h>
#include <vector2i.h>


///================================================
template<typename T>
class _Vector6T
{

public:
    T x, y, z;
	T x1, y1, z1;

    _Vector6T(){}

    inline _Vector6T(const T& X, const T& Y, const T& Z, const T& X1, const T& Y1, const T& Z1) 
	{ 
		x = X; y = Y; z = Z; 
		x1 = X1; y1 = Y1; z1 = Z1; 
	}

    inline explicit _Vector6T(const T& X) 
	{ 
		x = y = z = X; 
		x1 = y1 = z1 = X; 
	}

    inline _Vector6T(const _Vector6T& v) 
	{ 
		x = v.x; y = v.y; z = v.z; 
		x1 = v.x1; y1 = v.y1; z1 = v.z1; 
	}

	inline void BubbleSort(void)
	{
		_bubbleSortInt(&x, sizeof(_Vector6T)/sizeof(T));
	}

    inline T& operator[] (const int i) 
	{
		T *p = &x;
		return p[i];
    }
	
    inline const T& operator[] (const int i) const 
	{
		const T *p = &x;
		return p[i];
    }

    inline _Vector6T& operator += (const _Vector6T& A)
	{
		x += A.x;  y += A.y;  z += A.z;
		x1 += A.x1;  y1 += A.y1;  z1 += A.z1;
		return *this;
	}

    inline _Vector6T& operator *= (const T& A)
	{
		x *= A;  y *= A;  z *= A;
		x1 *= A;  y1 *= A;  z1 *= A;
		return *this;
	}

    inline _Vector6T& operator /= (const T& k)
	{
		x /= k;  y /= k;  z /= k;
		x1 /= k;  y1 /= k;  z1 /= k;
		return *this;
	}

    // Binary operators======================================
    inline friend _Vector6T operator + (const _Vector6T& A, const _Vector6T& B)
	{
		return _Vector6T(A.x+B.x, A.y+B.y, A.z+B.z, A.x1+B.x1, A.y1+B.y1, A.z1+B.z1);
	}

    inline friend _Vector6T operator - (const _Vector6T& A, const _Vector6T& B)
	{
		return _Vector6T(A.x-B.x, A.y-B.y, A.z-B.z, A.x1-B.x1, A.y1-B.y1, A.z1-B.z1);
	}
};


//=======vector6x definition=======================
typedef _Vector6T<double> Vector6d;
typedef _Vector6T<float> Vector6f;
typedef _Vector6T<int> Vector6i;


#endif
