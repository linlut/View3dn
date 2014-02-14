//FILE: vector3d.h
#ifndef __NZ__Vector3T__
#define __NZ__Vector3T__

#include <math.h>
#include <float.h>

template<class T> 
class _Vector3T
{
private:
	inline void _bubbleSortT(T a[], const int n)
	{
		const int n1 = n - 1;
		for (int i=0; i<n1; ++i) {
			for (int j=0; j<n1-i; ++j){
				if (a[j+1] < a[j]){  
					T tmp = a[j]; a[j] = a[j+1]; a[j+1] = tmp;
				}
			}
		}
	}

public:
    T x, y, z;

    // constructors
    _Vector3T(){ }

	inline _Vector3T(const T& X, const T& Y, const T& Z):
		x(X), y(Y), z(Z) {}

	explicit inline _Vector3T(const T& X) 
	{ 
		x = y = z = X;
	}

    inline _Vector3T(const _Vector3T& v) 
	{ 
		x = v.x; y = v.y; z = v.z; 
	}

	inline void bubbleSort(void)
	{
		_bubbleSortT(&x, 3);
	}
	//
	inline bool isValid(void)
	{
		T x0, y0, z0;
		x0 =x; y0 =y; z0 =z;
		if (_finite(x0)&& _finite(y0)&& _finite(z0))
			return true;
		return false;
	}

    // Functions to get at the vector components
    inline T& operator[] (const int& i)  
	{
		T *p = &x;
		return p[i];
    }

    inline const T& operator[] (const int& i) const
	{
		const T* p = &x;
		return p[i];
    }

	inline T norm(void) const
	{
		const double m2 = x*x + y*y + z*z;
		const double r = sqrt(m2);
		return (T)r;
	}

	inline T norm2(void) const
	{
		return x*x + y*y + z*z;
	}
	//
	inline void normalize(void)
	{
		const double m2 = x*x + y*y + z*z;
		const double r = sqrt(m2);
		const T r1 = (T)(1.0/r);
		x *= r1; y *= r1; z *= r1;		
	}

    // Unary operators
    inline _Vector3T operator + (void) const
	{
		return *this;
	}

    inline _Vector3T operator - (void) const
	{
		return _Vector3T(-x, -y, -z);
	}

    // Assignment operators
    inline _Vector3T& operator += (const _Vector3T& A)
	{
		x += A.x;  y += A.y;  z += A.z;
		return *this;
	}

    inline _Vector3T& operator -= (const _Vector3T& A)
	{
		x -= A.x;  y -= A.y;  z -= A.z;
		return *this;
	}

    inline _Vector3T& operator *= (const _Vector3T& A)
	{
		x *= A.x;  y *= A.y;  z *= A.z;
		return *this;
	}

    inline _Vector3T& operator *= (const T& k)
	{
		x *= k;  y *= k;  z *= k;
		return *this;
	}

    inline _Vector3T& operator /= (const T& k)
	{
		x /= k;  y /= k;  z /= k;
		return *this;
	}

    // Binary operators
    inline friend _Vector3T operator + (const _Vector3T& A, const _Vector3T& B)
	{
		return _Vector3T(A.x + B.x, A.y + B.y, A.z + B.z);
	}

    inline friend _Vector3T operator - (const _Vector3T& A, const _Vector3T& B)
	{
		return _Vector3T(A.x - B.x, A.y - B.y, A.z - B.z);
	}

    inline friend T operator * (const _Vector3T& A, const _Vector3T& B)
	{
		return DotProd(A, B);
	}

    inline friend _Vector3T operator * (const _Vector3T& A, const T& B)
	{
		return _Vector3T(A.x*B, A.y*B, A.z*B);
	}

    inline friend _Vector3T operator * (const T& a, const _Vector3T& B)
	{
		return _Vector3T(a*B.x, a*B.y, a*B.z);
	}

    //friend _Vector3T operator* (const Matrix&, const _Vector3T&);
    inline friend _Vector3T operator / (const _Vector3T& A, const _Vector3T& B)
	{
		return _Vector3T(A.x/B.x, A.y/B.y, A.z/B.z);
	}

    inline friend bool operator< (const _Vector3T& A, const _Vector3T& B)
	{
		return (A.x < B.x) && (A.y < B.y) && (A.z < B.z);
	}

	//===============for compatibility============================
	inline void setValue(const T& x0, const T& y0, const T& z0)
	{ 
		x=x0, y=y0, z=z0; 
	}
	inline void setValue(const _Vector3T& v)
	{ 
		x=v.x, y=v.y, z=v.z; 
	}
	inline void SetValue(const T& x0, const T& y0, const T& z0)
	{ 
		x=x0, y=y0, z=z0; 
	}
	inline void SetValue(const _Vector3T& v)
	{ 
		x=v.x, y=v.y, z=v.z; 
	}

	/*! Projects the Vec on the axis of direction \p direction that passes through the origin.
	\p direction does not need to be normalized (but must be non null). */
	void projectOnAxis(const _Vector3T& direction)
	{
		*this = (DotProd((*this), direction) / direction.norm2()) * direction;
	}

	/*! Projects the Vec on the plane whose normal is \p normal that passes through the origin.
	\p normal does not need to be normalized (but must be non null). */
	void projectOnPlane(const _Vector3T& normal)
	{
		(*this) -= (DotProd((*this), normal) / normal.norm2()) * normal;
	}

	//************************************************
    inline friend _Vector3T operator / (const _Vector3T& A, const T& B)
	{
		const T k = (T)(1.0 / B);
		return _Vector3T(A.x*k, A.y*k, A.z*k);
	}

	//Might replace floating-point == with comparisons of magnitudes, if needed.
    inline friend bool operator == (const _Vector3T& A, const _Vector3T& B)
	{
		return ((A.x == B.x) && (A.y == B.y) && (A.z == B.z));
	}

    inline friend T DotProd(const _Vector3T& A, const _Vector3T& B)
	{
		return (T)(A.x * B.x + A.y * B.y + A.z * B.z);
	}
	//
    inline friend _Vector3T CrossProd (const _Vector3T& A, const _Vector3T& B)
	{
		const T x = A.y * B.z - A.z * B.y;
		const T y = A.z * B.x - A.x * B.z;
		const T z = A.x * B.y - A.y * B.x;
		return _Vector3T(x, y, z);
	}
	//
    inline friend T Magnitude2(const _Vector3T& v)  
	{
		return (T)(v.x*v.x + v.y*v.y + v.z*v.z);
	}
	//
    inline friend T Magnitude(const _Vector3T& v)
	{
		const double r = v.x*v.x + v.y*v.y + v.z*v.z;
		return (T)(sqrt(r));
	}
	//
    inline friend T Distance(const _Vector3T& A, const _Vector3T& B)
	{
		const _Vector3T dis = A - B;
		return Magnitude(dis);
	}
	//
	inline friend T Distance2(const _Vector3T& A, const _Vector3T& B)
	{
		const _Vector3T dis = A - B;
		return Magnitude2(dis);
	}
	//
    inline friend _Vector3T Normalize(const _Vector3T& A)
	{
		_Vector3T r(A); r.normalize();
		return r;
	}

    //Rotate a normal vector.
    //friend _Vector3T PlaneRotate(const Matrix&, const _Vector3T&);

    //friend ostream& operator<< (const ostream& s, const _Vector3T& A);

    inline friend void Minimize(_Vector3T& minf, const _Vector3T& Candidate)
	{
		if (minf.x > Candidate.x ) minf.x = Candidate.x;
		if (minf.y > Candidate.y ) minf.y = Candidate.y;
		if (minf.z > Candidate.z ) minf.z = Candidate.z;
	}

    inline friend void Maximize(_Vector3T& maxf, const _Vector3T& Candidate)
	{
		if (maxf.x<Candidate.x) maxf.x = Candidate.x;
		if (maxf.y<Candidate.y) maxf.y = Candidate.y;
		if (maxf.z<Candidate.z) maxf.z = Candidate.z;
	}
};

typedef _Vector3T<double> Vector3d;

#endif
