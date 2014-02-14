/*****************************************************
FILE: vector2f.h
Author: Nan Zhang
Date: Aug 20, 2007

  define floating point triple tupple (x,y) as Vector2f
*****************************************************/


#ifndef __ZVECTOR2f__
#define __ZVECTOR2f__

#include <math.h>
#include <float.h>


class Vector2f {

public:
    float x, y;

    // constructors
    Vector2f() { }
    Vector2f(const float X, const float Y) { x = X; y = Y; }
    Vector2f(const float X) { x = y = X; }
    Vector2f(const Vector2f& v) { x = v.x; y = v.y; }

    // Functions to get at the vector components
    float& operator[] (int inx) 
	{
		float *p = &x;
		return p[inx];
    }
	//
    const float& operator[] (int inx) const 
	{
		const float *p = &x;
		return p[inx];
    }
	//
	void Normalize(void)
	{
		const double TINY = 1e-38;
		const double r = sqrt(x*x+y*y) + TINY;
		const float r1 = (float)(1.0 / r);
		x *= r1; y *= r1;
	}
	
    // Unary operators
    Vector2f operator+ () const;
    Vector2f operator- () const;

    // Assignment operators
    Vector2f& operator+= (const Vector2f& A);
    Vector2f& operator-= (const Vector2f& A);
    Vector2f& operator*= (const Vector2f& A);
    Vector2f& operator*= (float A);
    Vector2f& operator/= (float A);

    // Binary operators
    friend Vector2f operator+ (const Vector2f& A, const Vector2f& B);
    friend Vector2f operator- (const Vector2f& A, const Vector2f& B);
    friend Vector2f operator* (const Vector2f& A, const Vector2f& B);
    friend Vector2f operator* (const Vector2f& A, float B);
    friend Vector2f operator* (float A, const Vector2f& B);
    friend Vector2f operator* (const Matrix2f&, const Vector2f&);
    friend Vector2f operator/ (const Vector2f& A, const Vector2f& B)
	{
	    return Vector2f(A.x / B.x, A.y / B.y);
	}

    friend int operator< (const Vector2f& A, const Vector2f& B);

	//************************************************
    friend Vector2f operator/ (const Vector2f& A, const float B)
	{
	    return Vector2f(A.x / B, A.y / B);
	}
    
    friend int operator== (const Vector2f& A, const Vector2f& B);

    friend float DotProd(const Vector2f& A, const Vector2f& B)
	{
	    return A.x * B.x + A.y * B.y;
	}

	//
    friend double Magnitude(const Vector2f& v)
	{
	    return (double)sqrt((double)(v.x*v.x + v.y*v.y));
	}
    
	//
    friend double Magnitude2(const Vector2f& v)
	{
	    return (double)(v.x*v.x + v.y*v.y);
	}
    
	//
    friend float Distance(const Vector2f& A, const Vector2f&B)
    {
		const Vector2f dis = A-B;
		return (float)Magnitude(dis);
	}
    
	//
    friend float Distance2(const Vector2f& A, const Vector2f& B)
	{
		const Vector2f dis = A-B;
		const float r = dis.x*dis.x + dis.y *dis.y;
		return r;
	}

	//
    friend Vector2f Normalize(const Vector2f& A)
	{
		const double TINY = 1e-38;
		double t= Magnitude(A) + TINY;
		const float t1 = (float)(1.0 / t);
		return Vector2f(A.x*t1, A.y*t1);
	}
        
    //friend ostream& operator<< (const ostream& s, const Vector2f& A);

    friend void Minimize(Vector2f& min, const Vector2f& Candidate)
	{
		if (min.x > Candidate.x ) min.x = Candidate.x;
		if (min.y > Candidate.y ) min.y = Candidate.y;
	}

    friend void Maximize(Vector2f& max, const Vector2f& Candidate)
	{
		if (max.x<Candidate.x) max.x = Candidate.x;
		if (max.y<Candidate.y) max.y = Candidate.y;
	}
};

inline Vector2f Vector2f::operator+ () const
{
    return *this;
}

inline Vector2f Vector2f::operator- () const
{
    return Vector2f(-x, -y);
}

inline Vector2f& Vector2f::operator+= (const Vector2f& A)
{
    x += A.x;  y += A.y;;
    return *this;
}

inline Vector2f& Vector2f::operator-= (const Vector2f& A)
{
    x -= A.x;  y -= A.y;
    return *this;
}

inline Vector2f& Vector2f::operator*= (float A)
{
    x *= A;  y *= A;
    return *this;
}

inline Vector2f& Vector2f::operator/= (float A)
{
    x /= A;  y /= A;
    return *this;
}

inline Vector2f& Vector2f::operator*= (const Vector2f& A)
{
    x *= A.x;  y *= A.y;
    return *this;
}

inline Vector2f operator+ (const Vector2f& A, const Vector2f& B)
{
    return Vector2f(A.x + B.x, A.y + B.y);
}

inline Vector2f operator- (const Vector2f& A, const Vector2f& B)
{
    return Vector2f(A.x - B.x, A.y - B.y);
}

inline Vector2f operator* (const Vector2f& A, const Vector2f& B)
{
    return Vector2f(A.x * B.x, A.y * B.y);
}

inline Vector2f operator* (const Vector2f& A, float B)
{
    return Vector2f(A.x * B, A.y * B);
}

inline Vector2f operator* (float A, const Vector2f& B)
{
    return Vector2f(A * B.x + A * B.y);
}

inline int operator< (const Vector2f& A, const Vector2f& B)
{
    return (A.x < B.x) && (A.y < B.y);
}

// Might replace floating-point == with comparisons of
// magnitudes, if needed.
inline int operator== (const Vector2f& A, const Vector2f& B)
{
    return (A.x == B.x) && (A.y == B.y);
}


#endif