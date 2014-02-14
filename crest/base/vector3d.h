/*****************************************************
FILE: vector3d.h
Author: Nan Zhang
Date: Aug 20, 2007

  define double triple tupple (x,y,z) as Vector3d
*****************************************************/

#ifndef __NZ_VECTOR3D_H_
#define __NZ_VECTOR3D_H_


#include <math.h>
#include <float.h>


class Vector3d {

public:

    double x, y, z;

    // constructors
    Vector3d() { }
    Vector3d(const double& X, const double& Y, const double& Z) { x = X; y = Y; z = Z; }
    Vector3d(const double& X) { x = y = z = X; }
    Vector3d(const Vector3d& v) { x = v.x; y = v.y; z = v.z; }

	//
	int isValid(void)
	{
		double x0, y0, z0;
		x0 =x;
		y0 =y;
		z0 =z;
		if (_finite(x0)&& _finite(y0)&& _finite(z0))
			return true;
		return false;
	}


    // Functions to get at the vector components
    double & operator[] (const int i)  
	{
		double *p = &x;
		return p[i];
    }

    const double & operator[] (const int i) const
	{
		const double *p = &x;
		return p[i];
    }

	//
	void Normalize(void)
	{
		double r = sqrt(x*x+y*y+z*z);
		if (r == 0){
			x=1, y=0, z=0;
		}
		else{
			r = 1.0 / r;
			x *= r;
			y *= r;
			z *= r;
		}
	}
	//
    void ExtractVerts(double*px, double*py, int which) const;
	//
	void SetValue(const double x0, const double y0, const double z0)
	{ x =x0, y=y0, z=z0; }
	//
	void SetValue(const Vector3d & v)
	{ x =v.x, y=v.y, z=v.z; }

    // Unary operators
    Vector3d operator+ () const;
    Vector3d operator- () const;

    // Assignment operators
    Vector3d& operator+= (const Vector3d& A);
    Vector3d& operator-= (const Vector3d& A);
    Vector3d& operator*= (const Vector3d& A);
    Vector3d& operator*= (const double A);
    Vector3d& operator/= (const double A);

    // Binary operators
    friend Vector3d operator+ (const Vector3d& A, const Vector3d& B);
    friend Vector3d operator- (const Vector3d& A, const Vector3d& B);
    friend Vector3d operator* (const Vector3d& A, const Vector3d& B);
    friend Vector3d operator* (const Vector3d& A, const double B);
    friend Vector3d operator* (const double A, const Vector3d& B);
    //friend Vector3d operator* (const Matrix&, const Vector3d&);
    friend Vector3d operator/ (const Vector3d& A, const Vector3d& B);

    friend int operator< (const Vector3d& A, const Vector3d& B);

	//************************************************
    friend Vector3d operator/ (const Vector3d& A, const double B);

    friend int operator== (const Vector3d& A, const Vector3d& B);

    friend double DotProd(const Vector3d& A, const Vector3d& B);

    friend Vector3d CrossProd (const Vector3d& A, const Vector3d& B);
	//
    friend double Magnitude(const Vector3d& v);
	//
    friend double Magnitude2(const Vector3d& v);
	//
    friend double Distance(const Vector3d& A, const Vector3d&B);
	//
    friend double Distance2(const Vector3d& A, const Vector3d&B);

	//
    friend Vector3d Normalize(const Vector3d& A);
    // Rotate a normal vector.
    //friend Vector3d PlaneRotate(const Matrix&, const Vector3d&);

    //friend ostream& operator<< (const ostream& s, const Vector3d& A);

    friend void Minimize(Vector3d& min, const Vector3d& Candidate)
	{
		if (min.x > Candidate.x ) min.x = Candidate.x;
		if (min.y > Candidate.y ) min.y = Candidate.y;
		if (min.z > Candidate.z ) min.z = Candidate.z;
	}

    friend void Maximize(Vector3d& max, const Vector3d& Candidate)
	{
		if (max.x<Candidate.x) max.x = Candidate.x;
		if (max.y<Candidate.y) max.y = Candidate.y;
		if (max.z<Candidate.z) max.z = Candidate.z;
	}
};


inline double Magnitude(const Vector3d& v) 
{
    return (double)sqrt((double)(v.x*v.x + v.y*v.y + v.z*v.z));
}

inline double Magnitude2(const Vector3d& v) 
{
    return (double)(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline Vector3d Normalize(const Vector3d& A)
{
	Vector3d r = A;
	r.Normalize();
	return r;
}

//
inline double Distance(const Vector3d& A, const Vector3d&B)
{
	return Magnitude(A-B);
}

inline double Distance2(const Vector3d& A, const Vector3d&B)
{
	Vector3d dis = A-B;
	return dis.x*dis.x + dis.y *dis.y + dis.z*dis.z;
}


inline double DotProd (const Vector3d& A, const Vector3d& B)
{
    return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline Vector3d Vector3d::operator+ () const
{
    return *this;
}

inline Vector3d Vector3d::operator- () const
{
    return Vector3d(-x, -y, -z);
}

inline Vector3d& Vector3d::operator+= (const Vector3d& A)
{
    x += A.x;  y += A.y;  z += A.z;
    return *this;
}

inline Vector3d& Vector3d::operator-= (const Vector3d& A)
{
    x -= A.x;  y -= A.y;  z -= A.z;
    return *this;
}

inline Vector3d& Vector3d::operator*= (double A)
{
    x *= A;  y *= A;  z *= A;
    return *this;
}

inline Vector3d& Vector3d::operator/= (double A)
{
	const double k = 1.0/A;
    x *= k;  y *= k;  z *= k;
    return *this;
}

inline Vector3d& Vector3d::operator*= (const Vector3d& A)
{
    x *= A.x;  y *= A.y;  z *= A.z;
    return *this;
}

inline Vector3d operator+ (const Vector3d& A, const Vector3d& B)
{
    return Vector3d(A.x + B.x, A.y + B.y, A.z + B.z);
}

inline Vector3d operator- (const Vector3d& A, const Vector3d& B)
{
    return Vector3d(A.x - B.x, A.y - B.y, A.z - B.z);
}

inline Vector3d operator* (const Vector3d& A, const Vector3d& B)
{
    return Vector3d(A.x * B.x, A.y * B.y, A.z * B.z);
}

inline Vector3d operator* (const Vector3d& A, const double B)
{
    return Vector3d(A.x * B, A.y * B, A.z * B);
}

inline Vector3d operator* (const double a, const Vector3d& B)
{
	Vector3d n;
	n.x=a*B.x, n.y=a*B.y, n.z=a*B.z;
    return  n;
}

inline Vector3d operator/ (const Vector3d& A, const double B)
{
    return Vector3d(A.x / B, A.y / B, A.z / B);
}

inline Vector3d operator/ (const Vector3d& A, const Vector3d& B)
{
    return Vector3d(A.x / B.x, A.y / B.y, A.z / B.z);
}

inline int operator< (const Vector3d& A, const Vector3d& B)
{
    return A.x < B.x && A.y < B.y && A.z < B.z;
}

// Might replace floaing-point == with comparisons of
// magnitudes, if needed.
inline int operator== (const Vector3d& A, const Vector3d& B)
{
    return (A.x == B.x) && (A.y == B.y) && (A.z == B.z);
}

inline Vector3d CrossProd (const Vector3d& A, const Vector3d& B)
{
    return Vector3d(A.y * B.z - A.z * B.y,
					A.z * B.x - A.x * B.z,
					A.x * B.y - A.y * B.x);
}


#endif

