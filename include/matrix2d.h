// ===================================================================
//FILE: _Matrix2T.h
// ===================================================================

#ifndef ___Matrix2T_ZN2011__
#define ___Matrix2T_ZN2011__

#include <sysconf.h>
#include <vector3d.h>
#include <iostream>


// A 2x2 determinant
template<class T> 
T det2x2(const T& a, const T& b, const T& c, const T& d)
{
	const T r = a * d - b * c;
	return r;
}

// A 3x3 determinant
template<class T>
T det3x3(const T& a1, const T& a2, const T& a3,
		 const T& b1, const T& b2, const T& b3,
		 const T& c1, const T& c2, const T& c3)
{
	const T r =	  a1 * det2x2(b2, b3, c2, c3)
       			- b1 * det2x2(a2, a3, c2, c3)
       			+ c1 * det2x2(a2, a3, b2, b3);
	return r;
}

// Forward-declare some other classes
template<class T> class _Matrix2T
{
private:
    void _initWithValue(const T& val)
	{ 
		x[0][0]=x[0][1]=x[0][2]=x[0][3]=
		x[1][1]=x[1][0]=x[1][2]=x[1][3]=
		x[2][2]=x[2][0]=x[2][1]=x[2][3]=
		x[3][3]=x[3][0]=x[3][1]=x[3][2]=val;		
	}

	void _Adjoint(void)
	{
		T a1, a2, a3, a4, b1, b2, b3, b4;
		T c1, c2, c3, c4, d1, d2, d3, d4;

		/* assign to individual variable names to aid  */
		/* selecting correct values  */
		a1 = x[0][0]; b1 = x[0][1]; 
		c1 = x[0][2]; d1 = x[0][3];
		a2 = x[1][0]; b2 = x[1][1]; 
		c2 = x[1][2]; d2 = x[1][3];
		a3 = x[2][0]; b3 = x[2][1];
		c3 = x[2][2]; d3 = x[2][3];
		a4 = x[3][0]; b4 = x[3][1]; 
		c4 = x[3][2]; d4 = x[3][3];

		/* row column labeling reversed since we transpose rows & columns */
		x[0][0]  =   det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
		x[1][0]  = - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
		x[2][0]  =   det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
		x[3][0]  = - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
	        
		x[0][1]  = - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
		x[1][1]  =   det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
		x[2][1]  = - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
		x[3][1]  =   det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
	        
		x[0][2]  =   det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
		x[1][2]  = - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
		x[2][2]  =   det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
		x[3][2]  = - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
	        
		x[0][3]  = - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
		x[1][3]  =   det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
		x[2][3]  = - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
		x[3][3]  =   det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
	}

public:
    T x[4][4];

    _Matrix2T(void){ }

    inline explicit _Matrix2T(const T& val)
	{ 
		_initWithValue(val);
	}

    inline _Matrix2T(const T v[16])
	{ 
		x[0][0]=v[0],  x[0][1]=v[1],  x[0][2]=v[2],  x[0][3]=v[3];
		x[1][1]=v[4],  x[1][0]=v[5],  x[1][2]=v[6],  x[1][3]=v[7];
		x[2][2]=v[8],  x[2][0]=v[9],  x[2][1]=v[10], x[2][3]=v[11];
		x[3][3]=v[12], x[3][0]=v[13], x[3][1]=v[14], x[3][2]=v[15];		
	}

    ~_Matrix2T(){ }

	// determinant
	inline T det4x4(void) const
	{
		//assign to individual variable names to aid selecting correct elements 
		const T& a1 = x[0][0]; const T& b1 = x[0][1]; 
		const T& c1 = x[0][2]; const T& d1 = x[0][3];
		const T& a2 = x[1][0]; const T& b2 = x[1][1]; 
		const T& c2 = x[1][2]; const T& d2 = x[1][3];
		const T& a3 = x[2][0]; const T& b3 = x[2][1]; 
		const T& c3 = x[2][2]; const T& d3 = x[2][3];
		const T& a4 = x[3][0]; const T& b4 = x[3][1]; 
		const T& c4 = x[3][2]; const T& d4 = x[3][3];
		const T ans = a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
					- b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
					+ c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
					- d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
		return ans;
	}

	inline T det(void) const
	{
		return det4x4();
	}

	inline void setValue(const T & val)
	{ 
		SetValue(val);
	}

	//Arithmetic operations
    inline _Matrix2T& operator += (const _Matrix2T& A) 	// add-to
	{
		int i, j;
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				x[i][j] += A.x[i][j];
		return *this;
	}

    inline _Matrix2T& operator -= (const _Matrix2T& A) 	// subtract-from
	{
		int i, j;
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				x[i][j] -= A.x[i][j];
		return *this;
	}

    inline _Matrix2T& operator *= (const _Matrix2T& A) 	// multiply by _Matrix2T
	{
		_Matrix2T ret = *this;

		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++) {
				T subt = 0;
				for (int k = 0; k < 4; k++)
					subt += ret.x[i][k] * A.x[k][j];
				x[i][j] = subt;
			}
		}
		return *this;
	}

    inline _Matrix2T& operator *= (const T& A) 			// scale by scalar
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				x[i][j] *= A;
		return *this;
	}

    // Fundamental operations
    inline bool invert()				                // Invert the _Matrix2T
	{
		const T SMALL_NUMBER=1.e-30;
		const T det = det4x4();
		if ( det == 0) {
			//fprintf(stderr, "Singular matrix, no inverse!\n");
			return false;
		}

		_Adjoint();
		const T det1 = (T)(1.0)/det;
		T *p = &x[0][0];
		for (int i=0; i<16; ++i) p[i] *= det1;
		return true;
	}

	inline void transpose(void)			            // Transpose the _Matrix2T
	{
		for (int i = 0; i < 4; i++){
			for (int j = i; j < 4; j++){
				if (i != j) {
					const T temp = x[i][j];
					x[i][j] = x[j][i];
					x[j][i] = temp;
				}
			}
		}
	}

	// Dump
	void dump(void)
	{
		for (int i = 0; i < 4; i++){
			const T *p = this->x[i];
			printf("%d: %lg %lg %lg %lg\n", i, 
				(double)p[0], (double)p[1], (double)p[2], (double)p[3]);
		}
	}

    inline void setIdentityMatrix()
	{
		SetIdentityMatrix(*this);
	}

    inline void setZeroMatrix(void)
	{
	    SetZeroMatrix(*this);
	}

    inline void setScaleMatrix(const T& X, const T& Y, const T& Z)
	{
		SetScaleMatrix(*this, X, Y, Z);
	}

	//----------------------------------------------------------------------
    inline friend void SetTranslationMatrix(const _Vector3T<T>& Location, _Matrix2T &M)
	{
		M.setIdentityMatrix();
		M.x[3][0] = Location.x;
		M.x[3][1] = Location.y;
		M.x[3][2] = Location.z;
	}

    inline friend void SetRotationXMatrix(const T& Angle, _Matrix2T &M)
	{
		M.setIdentityMatrix();
		const T Cosine = cos(Angle);
		const T Sine = sin(Angle);
		M.x[1][1] = Cosine;
		M.x[2][1] = -Sine;
		M.x[1][2] = Sine;
		M.x[2][2] = Cosine;
	}

    inline friend void RotationYMatrix(const T& Angle, _Matrix2T &M)
	{
		SetIdentityMatrix(M);
		const T Cosine = cos(Angle);
		const T Sine = sin(Angle);
		M.x[0][0] = Cosine;
		M.x[2][0] = -Sine;
		M.x[0][2] = Sine;
		M.x[2][2] = Cosine;
	}

    inline friend void RotationZMatrix(const T& Angle, _Matrix2T &M)
	{
		SetIdentityMatrix(M);
		const T Cosine = cos(Angle);
		const T Sine = sin(Angle);
		M.x[0][0] = Cosine;
		M.x[1][0] = -Sine;
		M.x[0][1] = Sine;
		M.x[1][1] = Cosine;
	}

	// Construct a yaw-pitch-roll rotation _Matrix2T.	Rotate Yaw
	// radians about the XY axis, rotate Pitch radians in the
	// plane defined by the Yaw rotation, and rotate Roll radians
	// about the axis defined by the previous two angles.
    inline friend void RotationYPRMatrix(const T& Yaw, const T& Pitch, const T& Roll, _Matrix2T& M)
	{
		const double ch = cos(Yaw);
		const double sh = sin(Yaw);
		const double cp = cos(Pitch);
		const double sp = sin(Pitch);
		const double cr = cos(Roll);
		const double sr = sin(Roll);

		M.x[0][0] = ch*cr + sh*sp*sr;
		M.x[1][0] = -ch*sr + sh*sp*cr;
		M.x[2][0] = sh*cp;
		M.x[0][1] = sr*cp;
		M.x[1][1] = cr*cp;
		M.x[2][1] = -sp;
		M.x[0][2] = -sh*cr - ch*sp*sr;
		M.x[1][2] = sr*sh + ch*sp*cr;
		M.x[2][2] = ch*cp;
		for (int i = 0; i < 4; i++) M.x[3][i] = M.x[i][3] = 0;
		M.x[3][3] = 1;
	}

	// Construct a rotation of a given angle about a given axis.
	// Derived from Eric Haines's SPD (Standard Procedural Database).
    inline friend void RotationAxisMatrix(const _Vector3T<T>& axis, const T& angle, _Matrix2T& M)
	{
		const double cosine = cos(angle);
		const double sine = sin(angle);
		const double one_minus_cosine = 1 - cosine;

		M.x[0][0] = axis.x * axis.x + (1.0 - axis.x * axis.x) * cosine;
		M.x[0][1] = axis.x * axis.y * one_minus_cosine + axis.z * sine;
		M.x[0][2] = axis.x * axis.z * one_minus_cosine - axis.y * sine;
		M.x[0][3] = 0;

		M.x[1][0] = axis.x * axis.y * one_minus_cosine - axis.z * sine;
		M.x[1][1] = axis.y * axis.y + (1.0 - axis.y * axis.y) * cosine;
		M.x[1][2] = axis.y * axis.z * one_minus_cosine + axis.x * sine;
		M.x[1][3] = 0;

		M.x[2][0] = axis.x * axis.z * one_minus_cosine + axis.y * sine;
		M.x[2][1] = axis.y * axis.z * one_minus_cosine - axis.x * sine;
		M.x[2][2] = axis.z * axis.z + (1.0 - axis.z * axis.z) * cosine;
		M.x[2][3] = 0;

		M.x[3][0] = 0; M.x[3][1] = 0; M.x[3][2] = 0; M.x[3][3] = 1;
	}

	// Construct a scale _Matrix2T given the X, Y, and Z parameters
	// to scale by. To scale uniformly, let X==Y==Z.
    inline friend void SetScaleMatrix(_Matrix2T& M, const T& X, const T& Y, const T& Z)
	{
		SetIdentityMatrix(M);
		M.x[0][0] = X; M.x[1][1] = Y; M.x[2][2] = Z;
	}

    inline friend _Matrix2T Invert(const _Matrix2T& M) // Invert a given _Matrix2T
	{
		_Matrix2T InvertMe = M;
		InvertMe.invert();
		return InvertMe;
	}

    inline friend _Matrix2T TransposeMatrix(const _Matrix2T& M) // Transpose a given _Matrix2T
	{
		_Matrix2T TransposeMe = M;
		TransposeMe.transpose();
		return TransposeMe;
	}

    // Create various types of _Matrix2T.
    inline friend void SetIdentityMatrix(_Matrix2T &M)
	{
		M.x[0][0]=1, M.x[0][1]=M.x[0][2]=M.x[0][3]=0;
		M.x[1][1]=1, M.x[1][0]=M.x[1][2]=M.x[1][3]=0;
		M.x[2][2]=1, M.x[2][0]=M.x[2][1]=M.x[2][3]=0;
		M.x[3][3]=1, M.x[3][0]=M.x[3][1]=M.x[3][2]=0;
	}

    inline friend void SetZeroMatrix(_Matrix2T &M)
	{
		M._initWithValue(0);
	}

	// Construct a rotation matrix that makes the x, y, z axes
	// correspond to the vectors given.
    inline friend void GenRotationMatrix(const _Vector3T<T>& x, const _Vector3T<T>& y, const _Vector3T<T>& z, _Matrix2T &M)
	{
		SetIdentityMatrix(M);
		M.x[0][0] = x.x; M.x[1][0] = x.y; M.x[2][0] = x.z;
		M.x[0][1] = y.x; M.x[1][1] = y.y; M.x[2][1] = y.z;
		M.x[0][2] = z.x; M.x[1][2] = z.y; M.x[2][2] = z.z;
	}

	// Construct a view _Matrix2T to rotate and translate the world
	// given that the viewer is located at Viewer, looking at
	// LookAt, and the up vector is UpL.  The transformation
	// changes things around so the viewer is at the origin and
	// looking down the -Z axis.
    inline friend void ViewMatrix(const _Vector3T<T>& LookAt, const _Vector3T<T>& Viewer, const _Vector3T<T>& UpL, _Matrix2T &M)
	{
		SetIdentityMatrix(M);
		_Vector3T<T> U, V, N;
		_Vector3T<T> Up = Normalize(UpL);
		N = Normalize(Viewer - LookAt);
		V = Up - LookAt;
		V -= N * DotProd(V, N);
		V = Normalize(V);
		U = CrossProd(V, N);

		M.x[0][0] = U.x; M.x[1][0] = U.y; M.x[2][0] = U.z;
		M.x[0][1] = V.x; M.x[1][1] = V.y; M.x[2][1] = V.z;
		M.x[0][2] = N.x; M.x[1][2] = N.y; M.x[2][2] = N.z;

		M.x[3][0] = -DotProd(U, Viewer);
		M.x[3][1] = -DotProd(V, Viewer);
		M.x[3][2] = -DotProd(N, Viewer);
	}

	// Construct a quadric _Matrix2T.	After Foley et al. pp. 528-529.
    inline friend void QuadricMatrix(
		const T& a, const T& b, const T& c, const T& d, const T& e,
        const T& f, const T& g, const T& h, const T& j, const T& k, 
		_Matrix2T &M)
	{
		M.x[0][0] = a;  M.x[0][1] = d;  M.x[0][2] = f;  M.x[0][3] = g;
		M.x[1][0] = d;  M.x[1][1] = b;  M.x[1][2] = e;  M.x[1][3] = h;
		M.x[2][0] = f;  M.x[2][1] = e;  M.x[2][2] = c;  M.x[2][3] = j;
		M.x[3][0] = g;  M.x[3][1] = h;  M.x[3][2] = j;  M.x[3][3] = k;
	}

	// Construct various "mirror" matrices, which flip coordinate
	// signs in the various axes specified.
    inline friend void MirrorX(_Matrix2T &M)
	{
		SetIdentityMatrix(M);
		M.x[0][0] = -1;
	}

    inline friend void MirrorY(_Matrix2T &M)
	{
		SetIdentityMatrix(M);
		M.x[1][1] = -1;
	}

    inline friend void MirrorZ(_Matrix2T &M)
	{
		SetIdentityMatrix(M);
		M.x[2][2] = -1;
	}

    inline friend void RotationOnly(const _Matrix2T& x, _Matrix2T& M)
	{
		M = x;
		M.x[3][0] = M.x[3][1] = M.x[3][2] = 0;
	}

    // Binary operators
    inline friend _Matrix2T operator+ (const _Matrix2T& A, const _Matrix2T& B)
	{
		_Matrix2T ret;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				ret.x[i][j] = A.x[i][j] + B.x[i][j];
		return ret;
	}

    inline friend _Matrix2T operator- (const _Matrix2T& A, const _Matrix2T& B)
	{
		_Matrix2T ret;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				ret.x[i][j] = A.x[i][j] - B.x[i][j];
		return ret;
	}

    inline friend _Matrix2T operator * (const _Matrix2T& A, const T& b)
	{
		_Matrix2T M=A;
		M.x[0][0]*=b; M.x[0][1]*=b; M.x[0][2]*=b; M.x[0][3]*=b; 
		M.x[1][0]*=b; M.x[1][1]*=b; M.x[1][2]*=b; M.x[1][3]*=b; 
		M.x[2][0]*=b; M.x[2][1]*=b; M.x[2][2]*=b; M.x[2][3]*=b; 
		M.x[3][0]*=b; M.x[3][1]*=b; M.x[3][2]*=b; M.x[3][3]*=b; 
		return M;
	}

    inline friend _Matrix2T operator * (const _Matrix2T& A, const _Matrix2T& B)
	{
		_Matrix2T ret=A; ret*=B;
		return ret;
	}

	// Transform a vector by a _Matrix2T.
    inline friend _Vector3T<T> operator * (const _Vector3T<T>& v, const _Matrix2T& M)
	{
		_Vector3T<T> ret;
		ret.x = v.x * M.x[0][0] + v.y * M.x[1][0] + v.z * M.x[2][0] + M.x[3][0];
		ret.y = v.x * M.x[0][1] + v.y * M.x[1][1] + v.z * M.x[2][1] + M.x[3][1];
		ret.z = v.x * M.x[0][2] + v.y * M.x[1][2] + v.z * M.x[2][2] + M.x[3][2];
		const T denom = M.x[0][3] + M.x[1][3] + M.x[2][3] + M.x[3][3];
		if (denom != 1){
			const T denom1 = 1.0/denom;
			ret *= denom1;
		}
		return ret;
	}

    inline friend _Vector3T<T> operator * (const _Matrix2T& M, const _Vector3T<T>& v)
	{
		_Vector3T<T> ret;
		ret.x = v.x * M.x[0][0] + v.y * M.x[0][1] + v.z * M.x[0][2] + M.x[0][3];
		ret.y = v.x * M.x[1][0] + v.y * M.x[1][1] + v.z * M.x[1][2] + M.x[1][3];
		ret.z = v.x * M.x[2][0] + v.y * M.x[2][1] + v.z * M.x[2][2] + M.x[2][3];
		const T denom = M.x[3][0] + M.x[3][1] + M.x[3][2] + M.x[3][3];
		if (denom != 1){
			const T denom1 = 1.0/denom;
			ret *= denom1;
		}
		return ret;
	}

    inline friend _Vector3T<T> RotateOnly(const _Vector3T<T>& v, const _Matrix2T& M)
	{
		_Vector3T<T> ret;
		ret.x = v.x * M.x[0][0] + v.y * M.x[1][0] + v.z * M.x[2][0];
		ret.y = v.x * M.x[0][1] + v.y * M.x[1][1] + v.z * M.x[2][1];
		ret.z = v.x * M.x[0][2] + v.y * M.x[1][2] + v.z * M.x[2][2];
		return ret;
	}

	// Rotate a direction vector...
	inline friend _Vector3T<T> PlaneRotate(const _Matrix2T& tform, const _Vector3T<T>& p)
	{
		_Matrix2T m = tform;
		m.Invert();
		m.Transpose();
		// I hope that _Matrix2T is invertible...
		return RotateOnly(p, m);
	}

    // Overloaded output operator.
    inline friend std::ostream& operator << (std::ostream& s, const _Matrix2T& M)
	{
		for (int i=0; i<4; i++){
			const T * p = M.x[i];
			s<< p[0] << "," << p[1] << "," << p[2] << "," << p[3] <<std::endl;
		}
	}
};

typedef _Matrix2T<double> Matrix2d;
typedef _Matrix2T<double> double4x4;


#endif

