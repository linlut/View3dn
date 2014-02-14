/*****************************************************
FILE: vector2f.h
Author: Nan Zhang
Date: Aug 20, 2007
*****************************************************/

#ifndef __Z_Vector2T__
#define __Z_Vector2T__

#include <math.h>
#include <float.h>

template<class T> 
class _Vector2T {

public:
    T x, y;

    // constructors
    _Vector2T(){ }

    _Vector2T(const T X, const T Y) 
	{ 
		x = X; y = Y; 
	}

    _Vector2T(const _Vector2T& v)
	{ 
		x = v.x; y = v.y; 
	}

    // Functions to get at the vector components
    T& operator[] (const int inx) 
	{
		T* p = &x;
		return p[inx];
    }
	//
    const T& operator[] (const int inx) const 
	{
		const T* p = &x;
		return p[inx];
    }

	T& getX(void) 
	{
		return x;
	}

	const T& getX(void) const
	{
		return x;
	}

	T& getY(void)
	{
		return y;
	}

	const T& getY(void) const
	{
		return y;
	}

	//
	inline void normalize(void)
	{
		const double r = sqrt(x*x+y*y);
		const T r1 = (T)(1.0 / r);
		x *= r1; y *= r1;
	}
	
    // Unary operators
    inline _Vector2T operator + () const
	{
		return *this;
	}

    inline _Vector2T operator - () const
	{
		return _Vector2T(-x, -y);
	}

    // Assignment operators
    inline _Vector2T& operator += (const _Vector2T& A)
	{
		x += A.x;  y += A.y;;
		return *this;
	}

    inline _Vector2T& operator -= (const _Vector2T& A)
	{
		x -= A.x;  y -= A.y;
		return *this;
	}

    inline _Vector2T& operator *= (const _Vector2T& A)
	{
		x *= A.x;  y *= A.y;
		return *this;
	}

    inline _Vector2T& operator *= (const T& A)
	{
		x *= A;  y *= A;
		return *this;
	}

    inline _Vector2T& operator /= (const T& A)
	{
		x /= A;  y /= A;
		return *this;
	}

    // Binary operators
    inline friend _Vector2T operator+ (const _Vector2T& A, const _Vector2T& B)
	{
		return _Vector2T(A.x + B.x, A.y + B.y);
	}

    inline friend _Vector2T operator- (const _Vector2T& A, const _Vector2T& B)
	{
		return _Vector2T(A.x - B.x, A.y - B.y);
	}

    inline friend _Vector2T operator* (const _Vector2T& A, const _Vector2T& B)
	{
		return _Vector2T(A.x * B.x, A.y * B.y);
	}

    inline friend _Vector2T operator * (const _Vector2T& A, const T& B)
	{
		return _Vector2T(A.x * B, A.y * B);
	}

    inline friend _Vector2T operator * (const T& A, const _Vector2T& B)
	{
		return _Vector2T(A * B.x, A * B.y);
	}

    inline friend _Vector2T operator / (const _Vector2T& A, const _Vector2T& B)
	{
	    return _Vector2T(A.x / B.x, A.y / B.y);
	}

    inline friend int operator< (const _Vector2T& A, const _Vector2T& B)
	{
		return (A.x < B.x) && (A.y < B.y);
	}

    inline friend _Vector2T operator / (const _Vector2T& A, const T& B)
	{
	    return _Vector2T(A.x / B, A.y / B);
	}
    
    inline friend bool operator == (const _Vector2T& A, const _Vector2T& B)
	{
		return (A.x == B.x) && (A.y == B.y);
	}

    inline friend T DotProd(const _Vector2T& A, const _Vector2T& B)
	{
	    return A.x * B.x + A.y * B.y;
	}

	//
    inline friend T Magnitude(const _Vector2T& v)
	{
	    return (T)sqrt((double)(v.x*v.x + v.y*v.y));
	}
    
	//
    inline friend T Magnitude2(const _Vector2T& v)
	{
	    return (T)(v.x*v.x + v.y*v.y);
	}
    
	//
    inline friend T Distance(const _Vector2T& A, const _Vector2T&B)
    {
		const _Vector2T dis = A-B;
		return (T)Magnitude(dis);
	}
    
	//
    inline friend T Distance2(const _Vector2T& A, const _Vector2T& B)
	{
		const _Vector2T dis = A-B;
		const T r = dis.x*dis.x + dis.y *dis.y;
		return r;
	}

	//
    inline friend _Vector2T Normalize(const _Vector2T& A)
	{
		_Vector2T B = A; 
		B.normalize();
		return B;
	}
        
    //friend ostream& operator<< (const ostream& s, const _Vector2T& A);

    inline friend void Minimize(_Vector2T& minn, const _Vector2T& Candidate)
	{
		if (minn.x > Candidate.x ) minn.x = Candidate.x;
		if (minn.y > Candidate.y ) minn.y = Candidate.y;
	}

    friend void Maximize(_Vector2T& maxx, const _Vector2T& Candidate)
	{
		if (maxx.x<Candidate.x) maxx.x = Candidate.x;
		if (maxx.y<Candidate.y) maxx.y = Candidate.y;
	}
};


typedef _Vector2T<float> Vector2f;
typedef _Vector2T<double> Vector2d;


#endif