//FILE: vector7d.h

#ifndef __ZVECTOR7d_2012__
#define __ZVECTOR7d_2012__

#include <math.h>
#include <vector2i.h>


///================================================
template<typename T>
class _Vector7T
{

public:
    T x, y, z, w;
	T x1, y1, z1;

    _Vector7T(){}

    inline _Vector7T(const T& X, const T& Y, const T& Z, const T& W, const T& X1, const T& Y1, const T& Z1) 
	{ 
		x = X; y = Y; z = Z; w=W;
		x1 = X1; y1 = Y1; z1 = Z1; 
	}

    inline explicit _Vector7T(const T& X) 
	{ 
		x = y = z = w = X; 
		x1 = y1 = z1 = X; 
	}

    inline _Vector7T(const _Vector7T& v) 
	{ 
		x = v.x; y = v.y; z = v.z; w=v.w; 
		x1 = v.x1; y1 = v.y1; z1 = v.z1; 
	}

	inline void BubbleSort(void)
	{
		_bubbleSortInt(&x, sizeof(_Vector7T)/sizeof(T));
	}

    inline T& operator[] (int i) 
	{
		T *p = &x;
		return p[i];
    }
	
    inline const T& operator[] (int i) const 
	{
		const T *p = &x;
		return p[i];
    }

    inline _Vector7T& operator += (const _Vector7T& A)
	{
		x += A.x;  y += A.y;  z += A.z;  w+=A.w;
		x1 += A.x1;  y1 += A.y1;  z1 += A.z1;
		return *this;
	}

    inline _Vector7T& operator *= (const T& A)
	{
		x *= A;  y *= A;  z *= A;  w *= A;
		x1 *= A;  y1 *= A;  z1 *= A;
		return *this;
	}

    inline _Vector7T& operator /= (const T& k)
	{
		x /= k;  y /= k;  z /= k; w /= k;
		x1 /= k;  y1 /= k;  z1 /= k;
		return *this;
	}

    // Binary operators======================================
    inline friend _Vector7T operator + (const _Vector7T& A, const _Vector7T& B)
	{
		return _Vector7T(A.x+B.x, A.y+B.y, A.z+B.z, A.w+B.w, A.x1+B.x1, A.y1+B.y1, A.z1+B.z1);
	}

    inline friend _Vector7T operator - (const _Vector7T& A, const _Vector7T& B)
	{
		return _Vector7T(A.x-B.x, A.y-B.y, A.z-B.z, A.w-B.w, A.x1-B.x1, A.y1-B.y1, A.z1-B.z1);
	}
};


//=======vector6x definition=======================
typedef _Vector7T<double> Vector7d;
typedef _Vector7T<float> Vector7f;
typedef _Vector7T<int> Vector7i;


#endif
