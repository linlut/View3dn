// ===================================================================
// matrix3x3.h
// ===================================================================

#ifndef __MATRIX3x3_gagsa

#define __MATRIX3x3_gagsa

#include <sysconf.h>
#include <vector3d.h>
#include <vector3f.h>

template<class T> 
class Matrix3x3
{
public:
    T x[9];		//Data member to store the 9 coefficients of a 3x3 matrix

private: 
	typedef _Vector3T<T> Vec3T;

	inline void	_initWithValue(const T& val)
	{
		x[0]=x[1]=x[2]=x[3]=x[4]=x[5]=x[6]=x[7]=x[8]=val;
	}

	//comput matrix F^T * F in a fast way
	inline static void _FtF(const T f[9], T r[9])
	{
		r[0] = f[0]*f[0] + f[3]*f[3] + f[6]*f[6];
		r[1] = f[0]*f[1] + f[3]*f[4] + f[6]*f[7];
		r[2] = f[0]*f[2] + f[3]*f[5] + f[6]*f[8];
		//symetric
		r[3] = r[1]; 
		r[4] = f[1]*f[1] + f[4]*f[4] + f[7]*f[7];
		r[5] = f[1]*f[2] + f[4]*f[5] + f[7]*f[8];
		r[6] = r[2];
		r[7] = r[5];
		r[8] = f[2]*f[2] + f[5]*f[5] + f[8]*f[8];
	}

	inline static void _matMult(const T A[9], const T B[9], T C[9])
	{
		//first row
		C[0] = A[0]*B[0] + A[1]*B[3] + A[2]*B[6];  
		C[1] = A[0]*B[1] + A[1]*B[4] + A[2]*B[7];  
		C[2] = A[0]*B[2] + A[1]*B[5] + A[2]*B[8];  
		//second row
		C[3] = A[3]*B[0] + A[4]*B[3] + A[5]*B[6];  
		C[4] = A[3]*B[1] + A[4]*B[4] + A[5]*B[7];  
		C[5] = A[3]*B[2] + A[4]*B[5] + A[5]*B[8];  
		//third row
		C[6] = A[6]*B[0] + A[7]*B[3] + A[8]*B[6];  
		C[7] = A[6]*B[1] + A[7]*B[4] + A[8]*B[7];  
		C[8] = A[6]*B[2] + A[7]*B[5] + A[8]*B[8];
	}

	inline static void _matMultBRow0(const T A[9], const T B[9], T C[9])
	{
		//first row of C
		C[0] = A[0]*B[0]; C[1] = A[0]*B[1]; C[2] = A[0]*B[2];  
		//second row of C
		C[3] = A[3]*B[0]; C[4] = A[3]*B[1]; C[5] = A[3]*B[2];
		//third row of C
		C[6] = A[6]*B[0]; C[7] = A[6]*B[1]; C[8] = A[6]*B[2];
	}

	inline static void _matMultBRow1(const T A[9], const T B[9], T C[9])
	{
		//first row
		C[0] = A[1]*B[3]; C[1] = A[1]*B[4]; C[2] = A[1]*B[5];  
		//second row
		C[3] = A[4]*B[3]; C[4] = A[4]*B[4]; C[5] = A[4]*B[5];  
		//third row
		C[6] = A[7]*B[3]; C[7] = A[7]*B[4]; C[8] = A[7]*B[5];
	}

	inline static void _matMultBRow2(const T A[9], const T B[9], T C[9])
	{
		//first row
		C[0] = A[2]*B[6]; C[1] = A[2]*B[7]; C[2] = A[2]*B[8];  
		//second row
		C[3] = A[5]*B[6]; C[4] = A[5]*B[7]; C[5] = A[5]*B[8];  
		//third row
		C[6] = A[8]*B[6]; C[7] = A[8]*B[7]; C[8] = A[8]*B[8];
	}

public:
    Matrix3x3(void){ }

    inline explicit Matrix3x3(const T& val)
	{ 
		_initWithValue(val);
	}

    inline explicit Matrix3x3(const float m[9])
	{ 
		x[0]=m[0], x[1]=m[1], x[2]=m[2];
		x[3]=m[3], x[4]=m[4], x[5]=m[5];
		x[6]=m[6], x[7]=m[7], x[8]=m[8];
	}

    inline explicit Matrix3x3(const double m[9])
	{ 
		x[0]=m[0], x[1]=m[1], x[2]=m[2];
		x[3]=m[3], x[4]=m[4], x[5]=m[5];
		x[6]=m[6], x[7]=m[7], x[8]=m[8];
	}

    inline explicit Matrix3x3(const T m[4][4])
	{ 
		x[0]=m[0][0], x[1]=m[0][1], x[2]=m[0][2];
		x[3]=m[1][0], x[4]=m[1][1], x[5]=m[1][2];
		x[6]=m[2][0], x[7]=m[2][1], x[8]=m[2][2];
	}

	inline void setValue(const T & val)
	{ 
		SetValue(val);
	}

	inline void setZeroMatrix(void)
	{
		_initWithValue(0);
	}

	inline void setIdentityMatrix(void)
	{
		x[1]=x[2]=x[3]=x[5]=x[6]=x[7]=0;
		x[0]=x[4]=x[8]=1;
	}

    // get memember var, the starting index is 0, which follows C convension
    inline T& operator [] (const int i)
    {
		ASSERT0(i>=0 && i<9);
		return x[i];
    }

    // get memember var, the starting index is 1, which follows FORTRAN convension
    inline T& operator () (const int i)
    {
		ASSERT0(i>=1 && i<=9);
		return x[i-1];
    }

    inline Matrix3x3& operator += (const Matrix3x3& A)
    {
		const T *q = A.x;
		x[0]+=q[0], x[1]+=q[1], x[2]+=q[2], 
		x[3]+=q[3], x[4]+=q[4], x[5]+=q[5], 
		x[6]+=q[6], x[7]+=q[7], x[8]+=q[8]; 
		return *this;
    }
    
    inline Matrix3x3& operator -= (const Matrix3x3& A)
    {
		const T *q = A.x;
		x[0]-=q[0], x[1]-=q[1], x[2]-=q[2]; 
		x[3]-=q[3], x[4]-=q[4], x[5]-=q[5]; 
		x[6]-=q[6], x[7]-=q[7], x[8]-=q[8];     	
		return *this;
    }
    
    inline Matrix3x3& operator *= (const Matrix3x3& B)
    {
    	Matrix3x3 A=*this;
		_matMult(A.x, B.x, this->x);
		return *this;
    }
    
    inline Matrix3x3& operator *= (const T& A)
    {
		x[0]*=A, x[1]*=A, x[2]*=A,     	
		x[3]*=A, x[4]*=A, x[5]*=A,     	
		x[6]*=A, x[7]*=A, x[8]*=A;
		return *this;
    }

    inline void Transpose(void)
    {
		T tmp;
		tmp = x[1], x[1] = x[3], x[3] = tmp;
		tmp = x[2], x[2] = x[6], x[6] = tmp;
		tmp = x[5], x[5] = x[7], x[7] = tmp;
    }

	// Add the identity matrix to itself
    inline void AddIdentity(void)				
	{
		x[0]+=1; x[4]+=1; x[8]+=1;
	}

	// Minus the identity matrix to itself
    inline void MinusIdentity(void)				
	{
		x[0]-=1; x[4]-=1; x[8]-=1;
	}

	// Add the transpose of itself to itself
    inline void AddTranspose(void)				
	{
		x[0]+=x[0]; x[4]+=x[4]; x[8]+=x[8];
		x[1]+=x[3]; x[3]=x[1];
		x[2]+=x[6]; x[6]=x[2];
		x[5]+=x[7]; x[7]=x[5];
	}

	inline T Det(void) const
	{
		const T a11=x[0], a12=x[1], a13=x[2];
		const T a21=x[3], a22=x[4], a23=x[5];
		const T a31=x[6], a32=x[7], a33=x[8];
		const T r = a11*(a33*a22-a32*a23)-a21*(a33*a12-a32*a13)+a31*(a23*a12-a22*a13);
		return r;
	}

	inline T Trace(void) const
	{
		return x[0] + x[4] + x[8];
	}

	inline T FrobeniusNorm(void) const
	{
		T sum=x[0]*x[0]+x[1]*x[1]+x[2]*x[2];
		sum += x[3]*x[3]+x[4]*x[4]+x[5]*x[5];
		sum += x[6]*x[6]+x[7]*x[7]+x[8]*x[8];
		return (T)(sqrt((double)sum));
	}

    // Fundamental operations
    inline void Invert(void)				                // Invert the Matrix3x3
	{
		//The inverse of a 2x2 matrix:
		//| a11 a12 |-1             |  a22 -a12 |
		//| a21 a22 |    =  1/DET * | -a21  a11 |
		//with DET  =  a11a22-a12a21

		//The inverse of a 3x3 matrix:
		//| a11 a12 a13 |-1             |   a33a22-a32a23  -(a33a12-a32a13)   a23a12-a22a13  |
		//| a21 a22 a23 |    =  1/DET * | -(a33a21-a31a23)   a33a11-a31a13  -(a23a11-a21a13) |
		//| a31 a32 a33 |               |   a32a21-a31a22  -(a32a11-a31a12)   a22a11-a21a12  |
		//with DET  =  a11(a33a22-a32a23)-a21(a33a12-a32a13)+a31(a23a12-a22a13)
		double delta = this->Det();
		const double ZEROTOL = 1e-40;
		if (fabs(delta) < ZEROTOL){
			if (delta>0) 
				delta += ZEROTOL;
			else
				delta -= ZEROTOL;
		}
		const T invdelta = (T)(1.0/delta);
		const T a11=x[0], a12=x[1], a13=x[2];
		const T a21=x[3], a22=x[4], a23=x[5];
		const T a31=x[6], a32=x[7], a33=x[8];
		x[0] = (a33*a22-a32*a23)*invdelta;
		x[1] = -(a33*a12-a32*a13)*invdelta;
		x[2] = (a23*a12-a22*a13)*invdelta;
		x[3] = -(a33*a21-a31*a23)*invdelta;
		x[4] = (a33*a11-a31*a13)*invdelta;
		x[5] = -(a23*a11-a21*a13)*invdelta;
		x[6] = (a32*a21-a31*a22)*invdelta;
		x[7] = -(a32*a11-a31*a12)*invdelta;
		x[8] = (a22*a11-a21*a12)*invdelta;
	}

	inline T computeI1(void) const
	{
		return x[0]+x[4]+x[8]-3;
	}

	//==========friend functions====================
    inline friend void SetIdentityMatrix(Matrix3x3 &M)
	{
		T *x = M.x;
		x[1]=x[2]=x[3]=x[5]=x[6]=x[7]=0;
		x[0]=x[4]=x[8]=1;
	}

    inline friend void SetZeroMatrix(Matrix3x3 &M)
	{
		T *x = M.x;
		x[0]=x[1]=x[2]=
		x[3]=x[4]=x[5]=
		x[6]=x[7]=x[8]=0;
	}

	inline friend Matrix3x3 operator - (const Matrix3x3& A)
    {
		Matrix3x3 t;
		T* tx = t.x;
		const T* x = A.x;
		tx[0]=-x[0], tx[1]=-x[1], tx[2]=-x[2];
		tx[3]=-x[3], tx[4]=-x[4], tx[5]=-x[5];
		tx[6]=-x[6], tx[7]=-x[7], tx[8]=-x[8];
		return t;
    }

	inline friend Matrix3x3 operator + (const Matrix3x3& A, const Matrix3x3& B)
	{
		Matrix3x3 r;
		T* rx = r.x;
		const T* Ax = A.x;
		const T* Bx = B.x;
		rx[0] = Ax[0]+Bx[0]; rx[1] = Ax[1]+Bx[1]; rx[2] = Ax[2]+Bx[2];
		rx[3] = Ax[3]+Bx[3]; rx[4] = Ax[4]+Bx[4]; rx[5] = Ax[5]+Bx[5];
		rx[6] = Ax[6]+Bx[6]; rx[7] = Ax[7]+Bx[7]; rx[8] = Ax[8]+Bx[8];
		return r;
	}

	inline friend Matrix3x3 operator - (const Matrix3x3& A, const Matrix3x3& B)
	{
		Matrix3x3 r;
		T* rx = r.x;
		const T* Ax = A.x;
		const T* Bx = B.x;
		rx[0] = Ax[0]-Bx[0]; rx[1] = Ax[1]-Bx[1]; rx[2] = Ax[2]-Bx[2];
		rx[3] = Ax[3]-Bx[3]; rx[4] = Ax[4]-Bx[4]; rx[5] = Ax[5]-Bx[5];
		rx[6] = Ax[6]-Bx[6]; rx[7] = Ax[7]-Bx[7]; rx[8] = Ax[8]-Bx[8];
		return r;
	}

    inline friend Vec3T operator * (const Vec3T& t, const Matrix3x3<T>& rot)
	{
		Vec3T r;
		const T* rotx  = rot.x;
		r.x = rotx[0]*t.x + rotx[3]*t.y + rotx[6]*t.z;
		r.y = rotx[1]*t.x + rotx[4]*t.y + rotx[7]*t.z;
		r.z = rotx[2]*t.x + rotx[5]*t.y + rotx[8]*t.z;
		return r;		
	}

    inline friend Vec3T operator * (const Matrix3x3& rot, const Vec3T& t)
	{
		Vec3T r;
		const T * rotx  = rot.x;
		r.x = rotx[0]*t.x + rotx[1]*t.y + rotx[2]*t.z;
		r.y = rotx[3]*t.x + rotx[4]*t.y + rotx[5]*t.z;
		r.z = rotx[6]*t.x + rotx[7]*t.y + rotx[8]*t.z;
		return r;
	}

    inline friend Matrix3x3 operator * (const Matrix3x3& rot, const T& t)
	{
		Matrix3x3 mat;
		T* r = mat.x;
		const T* s = rot.x;
		r[0] = s[0]*t, r[1] = s[1]*t, r[2] = s[2]*t;
		r[3] = s[3]*t, r[4] = s[4]*t, r[5] = s[5]*t;
		r[6] = s[6]*t, r[7] = s[7]*t, r[8] = s[8]*t;
		return mat;
	}

    inline friend Matrix3x3 operator * (const T& t, const Matrix3x3& rot)
	{
		return rot*t;
	}

    inline friend Vector3f operator* (const Matrix3x3& rot, const float t[3])
	{
		Vector3f r;
		const T * rotx  = rot.x;
		r.x = rotx[0]*t[0] + rotx[1]*t[1] + rotx[2]*t[2];
		r.y = rotx[3]*t[0] + rotx[4]*t[1] + rotx[5]*t[2];
		r.z = rotx[6]*t[0] + rotx[7]*t[1] + rotx[8]*t[2];
		return r;
	}

	inline friend Vector3d operator* (const Matrix3x3& rot, const double t[3])
	{
		Vector3d r;
		const T * rotx  = rot.x;
		r.x = rotx[0]*t[0] + rotx[1]*t[1] + rotx[2]*t[2];
		r.y = rotx[3]*t[0] + rotx[4]*t[1] + rotx[5]*t[2];
		r.z = rotx[6]*t[0] + rotx[7]*t[1] + rotx[8]*t[2];
		return r;
	}

	inline friend Matrix3x3 operator* (const Matrix3x3& A, const Matrix3x3& B)
	{
    	Matrix3x3 C;
		_matMult(A.x, B.x, C.x);
		return C;
	}

    inline friend Matrix3x3 operator / (const Matrix3x3& rot, const T& t)
	{
		return rot*(1.0/t);
	}

    inline friend Matrix3x3<T> TransposeMatrix(const Matrix3x3<T>& A)
    {
		Matrix3x3 B;
		const T *p = A.x;
		T *q = B.x;
		q[0]=p[0], q[4]=p[4], q[8]=p[8];
		q[1]=p[3], q[2]=p[6];
		q[3]=p[1], q[5]=p[7];
		q[6]=p[2], q[7]=p[5];
		return B;
    }

	inline friend Matrix3x3 Mult(const Matrix3x3& A, const Matrix3x3& B)
	{
		return A*B;
	}

	inline friend Matrix3x3 MultRow0(const Matrix3x3& A, const Matrix3x3& B)
	{
		Matrix3x3 C;
		_matMultBRow0(A.x, B.x, C.x);
		return C;
	}

	inline friend Matrix3x3 MultRow1(const Matrix3x3& A, const Matrix3x3& B)
	{
		Matrix3x3 C;
		_matMultBRow1(A.x, B.x, C.x);
		return C;
	}

	inline friend Matrix3x3 MultRow2(const Matrix3x3& A, const Matrix3x3& B)
	{
		Matrix3x3 C;
		_matMultBRow2(A.x, B.x, C.x);
		return C;
	}

	//Right Green-St Vernat strain
	inline friend Matrix3x3 FtF(const Matrix3x3& A)
	{
		Matrix3x3 B;
		_FtF(A.x, B.x);
		return B;
	}

	inline friend T InnerProd(const Matrix3x3& A, const Matrix3x3& B)
	{
		const T* x=A.x;
		const T* y=B.x;
		T r =	x[0]*y[0]+x[1]*y[1]+x[2]*y[2]+
				x[3]*y[3]+x[4]*y[4]+x[5]*y[5]+
				x[6]*y[6]+x[7]*y[7]+x[8]*y[8];
		return r;
	}

	inline friend Vec3T DotProd(const Vec3T& A, const Matrix3x3& B)
	{
		Vec3T r;
		const T* x = &A.x;
		const T* y = B.x;
		r.x = x[0]*y[0]+x[1]*y[1]+x[2]*y[2];  y+=3;
		r.y = x[0]*y[0]+x[1]*y[1]+x[2]*y[2];  y+=3;
		r.z = x[0]*y[0]+x[1]*y[1]+x[2]*y[2];
		return r;
	}

	//The tensor product operation between two 1D vectors
	// FIJ = DIJ X N
	inline friend void VectorTensorProduct(const Vec3T& DIJ, const Vec3T& N, Matrix3x3 &FIJ)
	{
		T *x = FIJ.x;
		x[0] = DIJ.x * N.x;
		x[1] = DIJ.x * N.y;
		x[2] = DIJ.x * N.z;
		x[3] = DIJ.y * N.x;
		x[4] = DIJ.y * N.y;
		x[5] = DIJ.y * N.z;
		x[6] = DIJ.z * N.x;
		x[7] = DIJ.z * N.y;
		x[8] = DIJ.z * N.z;
	}

	//Input matrices A and B, output matrix C=A^T*B
	inline friend void Matrix3x3TransposeMult(const Matrix3x3& MA, const Matrix3x3& MB, Matrix3x3& MC)
	{
		const T *A = MA.x;
		const T *B = MB.x;
		T *C = MC.x;
		//first row
		C[0] = A[0]*B[0] + A[3]*B[3] + A[6]*B[6];  
		C[1] = A[0]*B[1] + A[3]*B[4] + A[6]*B[7];  
		C[2] = A[0]*B[2] + A[3]*B[5] + A[6]*B[8];  
		//second row
		C[3] = A[1]*B[0] + A[4]*B[3] + A[7]*B[6];  
		C[4] = A[1]*B[1] + A[4]*B[4] + A[7]*B[7];  
		C[5] = A[1]*B[2] + A[4]*B[5] + A[7]*B[8];  
		//third row
		C[6] = A[2]*B[0] + A[5]*B[3] + A[8]*B[6];  
		C[7] = A[2]*B[1] + A[5]*B[4] + A[8]*B[7];  
		C[8] = A[2]*B[2] + A[5]*B[5] + A[8]*B[8];
	}

	inline friend void CopyVectorsToRows(const Vec3T &q1, const Vec3T &q2, const Vec3T &q3, Matrix3x3 &m)
	{
		m.x[0]=q1.x, m.x[1]=q1.y, m.x[2]=q1.z;
		m.x[3]=q2.x, m.x[4]=q2.y, m.x[5]=q2.z;
		m.x[6]=q3.x, m.x[7]=q3.y, m.x[8]=q3.z;
	}

	inline friend void CopyVectorsToColumns(const Vec3T &q1, const Vec3T &q2, const Vec3T &q3, Matrix3x3 &m)
	{
		m.x[0]=q1.x, m.x[3]=q1.y, m.x[6]=q1.z;
		m.x[1]=q2.x, m.x[4]=q2.y, m.x[7]=q2.z;
		m.x[2]=q3.x, m.x[5]=q3.y, m.x[8]=q3.z;
	}

};

typedef Matrix3x3<float> float3x3;
typedef Matrix3x3<double> double3x3;

#endif
