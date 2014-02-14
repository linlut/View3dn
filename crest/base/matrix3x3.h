// ===================================================================
// matrix3x3.h
// ===================================================================

#ifndef __MATRIX3x3_gagsa

#define __MATRIX3x3_gagsa

#include <base/sysconf.h>
#include <base/vectorall.h>


template<class T>
class Matrix3x3{

public:
    T x[3][3];

    Matrix3x3() { }

	void ZeroMatrix(void)
	{
		T* p = &x[0][0];
		p[0]=p[1]=p[2]=p[3]=p[4]=p[5]=p[6]=p[7]=p[8]=0;
	}

	void IdentityMatrix(void)
	{
		T* p = &x[0][0];
		p[1]=p[2]=p[3]=p[5]=p[6]=p[7]=0;
		p[0]=p[4]=p[8]=1;
	}

    // Assignment operators
    Matrix3x3& operator+= (const Matrix3x3& A)
    {
		T *p = &x[0][0];
		const T *q = &A.x[0][0];
		p[0]+=q[0], p[1]+=q[1], p[2]+=q[2], 
		p[3]+=q[3], p[4]+=q[4], p[5]+=q[5], 
		p[6]+=q[6], p[7]+=q[7], p[8]+=q[8]; 
    }
    
    Matrix3x3& operator-= (const Matrix3x3& A)
    {
		T *p = &x[0][0];
		const T *q = &A.x[0][0];
		p[0]-=q[0], p[1]-=q[1], p[2]-=q[2], 
		p[3]-=q[3], p[4]-=q[4], p[5]-=q[5], 
		p[6]-=q[6], p[7]-=q[7], p[8]-=q[8];     	
    }
    
    //Matrix3x3& operator*= (const Matrix3x3& A); 	// multiply by Matrix3x3
    
    Matrix3x3& operator*= (float A)					// scale by scalar
    {
		T *p = &x[0][0];
		p[0]*=A, p[1]*=A, p[2]*=A,     	
		p[3]*=A, p[4]*=A, p[5]*=A,     	
		p[6]*=A, p[7]*=A, p[8]*=A;
    }

    // Fundamental operations
    //void Invert();				                // Invert the Matrix3x3
    void Transpose()				            // Transpose the Matrix3x3
    {
		T tmp;
		tmp = x[0][1], x[0][1] = x[1][0], x[1][0] = tmp;
		tmp = x[0][2], x[0][2] = x[2][0], x[2][0] = tmp;
		tmp = x[2][1], x[2][1] = x[1][2], x[1][2] = tmp;
    }

	T Det(void) const
	{
		const T& a1=x[0][0];
		const T& b1=x[1][0];
		const T& c1=x[2][0];
		const T& a2=x[0][1];
		const T& b2=x[1][1];
		const T& c2=x[2][1];
		const T& a3=x[0][2];
		const T& b3=x[1][2];
		const T& c3=x[2][2];
		return a1*b2*c3 + b1*c2*a3 + c1*a2*b3 - a3*b2*c1 - b3*c2*a1 - c3*a2*b1;
	}

    //friend Matrix3x3 Invert(const Matrix3x3& M);	// Invert a given Matrix3x3
    //friend Matrix3x3 Transpose(const Matrix3x3& M);// Transpose a given Matrix3x3

    // Create various types of Matrix3x3.
    /*
    friend Matrix3x3 IdentityMatrix3x3();
    friend Matrix3x3 ZeroMatrix3x3();
    friend Matrix3x3 TranslationMatrix3x3(const Vector3f& Location);
    friend Matrix3x3 RotationXMatrix3x3(float Angle);
    friend Matrix3x3 RotationYMatrix3x3(float Angle);
    friend Matrix3x3 RotationZMatrix3x3(float Angle);
    friend Matrix3x3 RotationYPRMatrix3x3(float Yaw, float Pitch, float Roll);
    friend Matrix3x3 RotationAxisMatrix3x3(const Vector3f& axis, float Angle);

    friend Matrix3x3 ScaleMatrix3x3(float X, float Y, float Z);
    friend Matrix3x3 GenRotation2f(const Vector3f& x, const Vector3f& y, const Vector3f& z);
    friend Matrix3x3 ViewMatrix3x3(const Vector3f& LookAt, const Vector3f& Viewer, const Vector3f& Up);
    friend Matrix3x3 QuadricMatrix3x3(float a, float b, float c, float d, float e,
                    	        float f, float g, float h, float j, float k);
    friend Matrix3x3 MirrorX2f();
    friend Matrix3x3 MirrorY2f();
    friend Matrix3x3 MirrorZ2f();
    friend Matrix3x3 RotationOnly2f(const Matrix3x3& x);

    // Binary operators
    friend Matrix3x3 operator* (const Matrix3x3& A, float B);
    friend Matrix3x3 operator* (const Matrix3x3& A, const Matrix3x3& B);
    friend class Vector3f RotateOnly(const Vector3f& v, const Matrix3x3& M);
    friend class Vector3f RotateOnly(const Matrix3x3& M, const Vector3f& v)
    {
    	return RotateOnly(v, M);
    }

    // Overloaded output operator.
    //friend ostream& operator<< (ostream& s, const Matrix3x3& M);
    */

	friend Matrix3x3 operator+ (const Matrix3x3& A, const Matrix3x3& B)
	{
		Matrix3x3 r;
		r.x[0][0] = A.x[0][0]+B.x[0][0];
		r.x[0][1] = A.x[0][1]+B.x[0][1];
		r.x[0][2] = A.x[0][2]+B.x[0][2];
		r.x[1][0] = A.x[1][0]+B.x[1][0];
		r.x[1][1] = A.x[1][1]+B.x[1][1];
		r.x[1][2] = A.x[1][2]+B.x[1][2];
		r.x[2][0] = A.x[2][0]+B.x[2][0];
		r.x[2][1] = A.x[2][1]+B.x[2][1];
		r.x[2][2] = A.x[2][2]+B.x[2][2];
		return r;
	}

	friend Matrix3x3 operator- (const Matrix3x3& A, const Matrix3x3& B)
	{
		Matrix3x3 r;
		r.x[0][0] = A.x[0][0]-B.x[0][0];
		r.x[0][1] = A.x[0][1]-B.x[0][1];
		r.x[0][2] = A.x[0][2]-B.x[0][2];
		r.x[1][0] = A.x[1][0]-B.x[1][0];
		r.x[1][1] = A.x[1][1]-B.x[1][1];
		r.x[1][2] = A.x[1][2]-B.x[1][2];
		r.x[2][0] = A.x[2][0]-B.x[2][0];
		r.x[2][1] = A.x[2][1]-B.x[2][1];
		r.x[2][2] = A.x[2][2]-B.x[2][2];
		return r;
	}

    friend Vector3f operator* (const Vector3f& t, const Matrix3x3<T>& rot)
	{
		Vector3f r;
		r.x = rot.x[0][0]*t.x + rot.x[1][0]*t.y + rot.x[2][0]*t.z;
		r.y = rot.x[0][1]*t.x + rot.x[1][1]*t.y + rot.x[2][1]*t.z;
		r.z = rot.x[0][2]*t.x + rot.x[1][2]*t.y + rot.x[2][2]*t.z;
		return r;		
	}

    friend Vector3f operator* (const Matrix3x3<T>& rot, const Vector3f& t)
	{
		Vector3f r;
		r.x = rot.x[0][0]*t.x + rot.x[0][1]*t.y + rot.x[0][2]*t.z;
		r.y = rot.x[1][0]*t.x + rot.x[1][1]*t.y + rot.x[1][2]*t.z;
		r.z = rot.x[2][0]*t.x + rot.x[2][1]*t.y + rot.x[2][2]*t.z;
		return r;
	}

	friend Vector3d operator* (const Vector3d& t, const Matrix3x3<T>& rot)
	{
		Vector3d r;
		r.x = rot.x[0][0]*t.x + rot.x[1][0]*t.y + rot.x[2][0]*t.z;
		r.y = rot.x[0][1]*t.x + rot.x[1][1]*t.y + rot.x[2][1]*t.z;
		r.z = rot.x[0][2]*t.x + rot.x[1][2]*t.y + rot.x[2][2]*t.z;
		return r;		
	}

	friend Vector3d operator* (const Matrix3x3<T>& rot, const Vector3d& t)
	{
		Vector3d r;
		r.x = rot.x[0][0]*t.x + rot.x[0][1]*t.y + rot.x[0][2]*t.z;
		r.y = rot.x[1][0]*t.x + rot.x[1][1]*t.y + rot.x[1][2]*t.z;
		r.z = rot.x[2][0]*t.x + rot.x[2][1]*t.y + rot.x[2][2]*t.z;
		return r;
	}
	
};



typedef Matrix3x3<float> float3x3;
typedef Matrix3x3<double> double3x3;

#endif

