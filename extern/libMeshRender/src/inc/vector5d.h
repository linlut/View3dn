//FILE: vector5d.h

#ifndef __ZVECTOR5d_2012__
#define __ZVECTOR5d_2012__

#include <math.h>
#include <vector2i.h>


///================================================
template<typename T>
class _Vector5T
{

public:
    T x, y, z;
	T x1, y1;

    _Vector5T(){}

    inline _Vector5T(const T& X, const T& Y, const T& Z, const T& X1, const T& Y1) 
	{ 
		x = X; y = Y; z = Z;
		x1 = X1; y1=Y1; 
	}

    inline explicit _Vector5T(const T& X) 
	{ 
		x = y = z = X; 
		x1 = y1 = X; 
	}

    inline _Vector5T(const _Vector5T& v) 
	{ 
		x = v.x; y = v.y; z = v.z; 
		x1 = v.x1; y1 = v.y1; 
	}

	inline void BubbleSort(void)
	{
		_bubbleSortInt(&x, sizeof(_Vector5T)/sizeof(T));
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

    inline _Vector5T& operator += (const _Vector5T& A)
	{
		x += A.x;  y += A.y;  z += A.z;
		x1 += A.x1;  y1 += A.y1;
		return *this;
	}

    inline _Vector5T& operator *= (const T& A)
	{
		x *= A;  y *= A;  z *= A;
		x1 *= A;  y1 *= A;
		return *this;
	}

    inline _Vector5T& operator /= (const T& k)
	{
		x /= k;  y /= k;  z /= k;
		x1 /= k;  y1 /= k;
		return *this;
	}

    // Binary operators======================================
    inline friend _Vector5T operator + (const _Vector5T& A, const _Vector5T& B)
	{
		return _Vector5T(A.x+B.x, A.y+B.y, A.z+B.z, A.x1+B.x1, A.y1+B.y1);
	}

    inline friend _Vector5T operator - (const _Vector5T& A, const _Vector5T& B)
	{
		return _Vector5T(A.x-B.x, A.y-B.y, A.z-B.z, A.x1-B.x1, A.y1-B.y1);
	}
};


//=======vector5x definition=======================
typedef _Vector5T<double> Vector5d;
typedef _Vector5T<float> Vector5f;
typedef _Vector5T<int> Vector5i;


#endif
