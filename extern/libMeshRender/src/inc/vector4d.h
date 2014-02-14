/*****************************************************
FILE: vector4d.h
Author: Nan Zhang
Date: Jan 20, 2009

  define T quad tupple (x,y,z,w) as _Vector4T
*****************************************************/

#ifndef _NZ_Z_Vector4T__
#define _NZ_Z_Vector4T__
#include <math.h>


template<class T> 
class _Vector4T
{
private:
	inline void _bubbleSortT(T a[], const int n)
	{
		const int n1 = n-1;
		for (int i=0; i<n1; i++) {
			for (int j=0; j<n1-i; j++){
				if (a[j+1] < a[j]){  
					T tmp = a[j]; a[j] = a[j+1]; a[j+1] = tmp;
				}
			}
		}
	}

public:
    T x, y, z, w;

    // constructors
    _Vector4T(){ }

	inline _Vector4T(const T& X, const T& Y, const T& Z, const T& W):
		x(X), y(Y), z(Z), w(W) {}
    
	explicit inline _Vector4T(const T& X) 
	{ 
		x = y = z = w = X; 
	}
    
	inline _Vector4T(const _Vector4T& v) 
	{ 
		x = v.x; y = v.y; z = v.z; w=v.w; 
	}

    // Functions to get at the vector components
    inline T& operator[] (const int i) 
	{
		T *p = &x;
		return p[i];
    }
	//
    inline const T& operator[] (const int i) const 
	{
		const T *p = &x;
		return p[i];
    }
	//
	inline void normalize(void)
	{
		const double r = sqrt((double)(x*x+y*y+z*z+w*w));
		const T r1 = (T)(1.0 / r);
		x *= r1; y *= r1; z *= r1; w *= r1;
	}
	//
	inline void bubbleSort(void)
	{
		_bubbleSortT(&x, 4);
	}
	//
	inline void setValue(const T& x0, const T& y0, const T& z0, const T& w0)
	{ 
		x=x0, y=y0, z=z0, w=w0; 
	}
	//
	inline void setValue(const _Vector4T & v)
	{ 
		x=v.x, y=v.y, z=v.z, w=v.w; 
	}
    // Unary operators
    inline _Vector4T operator+ () const
	{
		return *this;
	}
	//
    inline _Vector4T operator - () const
	{
		return _Vector4T(-x, -y, -z, -w);
	}
    // Assignment operators
    inline _Vector4T& operator += (const _Vector4T& A)
	{
		x += A.x;  y += A.y;  z += A.z; w+=A.w;
		return *this;
	}
	//
    inline _Vector4T& operator -= (const _Vector4T& A)
	{
		x -= A.x;  y -= A.y;  z -= A.z; w -= A.w;
		return *this;
	}
	//
    inline _Vector4T& operator *= (const _Vector4T& A)
	{
		x *= A.x;  y *= A.y;  z *= A.z; w*=A.w;
		return *this;
	}
	//
    inline _Vector4T& operator *= (const T& A)
	{
		x *= A;  y *= A;  z *= A; w*=A;
		return *this;
	}
	//
    inline _Vector4T& operator /= (const T& A)
	{
		const T A1 = 1.0f/A;
		x *= A1;  y *= A1;  z *= A1;  w*= A1;
		return *this;
	}
	// Binary operators
    friend inline _Vector4T operator + (const _Vector4T& A, const _Vector4T& B)
	{
		return _Vector4T(A.x + B.x, A.y + B.y, A.z + B.z, A.w+B.w);
	}

    friend inline _Vector4T operator - (const _Vector4T& A, const _Vector4T& B)
	{
		return _Vector4T(A.x - B.x, A.y - B.y, A.z - B.z, A.w-B.w);
	}

    friend inline _Vector4T operator * (const _Vector4T& A, const _Vector4T& B)
	{
		return _Vector4T(A.x * B.x, A.y * B.y, A.z * B.z, A.w*B.w);
	}

    friend inline _Vector4T operator * (const _Vector4T& A, const T& B)
	{
		return _Vector4T(A.x * B, A.y * B, A.z * B, A.w*B);
	}
	//
    friend inline _Vector4T operator * (const T& A, const _Vector4T& B)
	{
		return B * A;
	}
	//
	friend inline _Vector4T operator /(const _Vector4T& A, const T& B)
	{
		const T B1=1.0f/B;
		return _Vector4T(A.x*B1, A.y*B1, A.z*B1, A.w*B1);
	}
	//
    friend inline _Vector4T operator / (const _Vector4T& A, const _Vector4T& B)
	{
		return _Vector4T(A.x/B.x, A.y/B.y, A.z/B.z, A.w/B.w);
	}

    friend inline bool operator < (const _Vector4T& A, const _Vector4T& B)
	{
		return (A.x < B.x) && (A.y < B.y) && (A.z < B.z) && (A.w< B.w);
	}
	//
    friend inline bool operator == (const _Vector4T& A, const _Vector4T& B)
	{
		return (A.x == B.x) && (A.y == B.y) && (A.z == B.z) && (A.w==B.w);
	}
	//
    friend inline T DotProd(const _Vector4T& A, const _Vector4T& B)
	{
		return (T)(A.x * B.x + A.y * B.y + A.z * B.z + A.w*B.w);
	}
	//
    friend inline T Magnitude2(const _Vector4T& v)
	{
		return (T)(v.x*v.x + v.y*v.y + v.z*v.z+v.w*v.w);
	}
	//
    friend T Magnitude(const _Vector4T& v)
	{
		return (T)sqrt((double)Magnitude2(v));
	}
	//
    friend inline T Distance2(const _Vector4T& A, const _Vector4T& B)
	{
		return (T)(Magnitude2(A-B));
	}
	//
    friend inline T Distance(const _Vector4T& A, const _Vector4T& B)
	{
		return sqrt((double)Distance2(A, B));
	}
	//
    friend inline _Vector4T Normalize(const _Vector4T& A)
	{
		_Vector4T B(A); 
		B.normalize();
		return B;
	}
	//
    inline friend void Minimize(_Vector4T& min, const _Vector4T& Candidate)
	{
		if (min.x > Candidate.x ) min.x = Candidate.x;
		if (min.y > Candidate.y ) min.y = Candidate.y;
		if (min.z > Candidate.z ) min.z = Candidate.z;
		if (min.w > Candidate.w ) min.w = Candidate.w;
	}
	//
    inline friend void Maximize(_Vector4T& max, const _Vector4T& Candidate)
	{
		if (max.x<Candidate.x) max.x = Candidate.x;
		if (max.y<Candidate.y) max.y = Candidate.y;
		if (max.z<Candidate.z) max.z = Candidate.z;
		if (max.w<Candidate.w) max.w = Candidate.w;
	}
};

typedef _Vector4T<double> Vector4d;

#endif