// ===================================================================
// matrix2f.h
// ===================================================================

#ifndef __MATRIX2F_234a80ag__

#define __MATRIX2F_234a80ag__


#include <crest/base/sysconf.h>
#include <crest/base/vectorall.h>


// Forward-declare some other classes

class Matrix2f {

public:
    float x[4][4];

    Matrix2f() { }

    // Assignment operators
    Matrix2f& operator+= (const Matrix2f& A); 	// add-to
    Matrix2f& operator-= (const Matrix2f& A); 	// subtract-from
    Matrix2f& operator*= (const Matrix2f& A); 	// multiply by Matrix2f
    Matrix2f& operator*= (float A); 		    // scale by scalar

    // Fundamental operations
    void Invert();				                // Invert the Matrix2f
    void Transpose();				            // Transpose the Matrix2f
    friend Matrix2f Invert(const Matrix2f& M);	// Invert a given Matrix2f
    friend Matrix2f Transpose(const Matrix2f& M);// Transpose a given Matrix2f

    // Create various types of Matrix2f.
    friend Matrix2f IdentityMatrix2f();
    friend Matrix2f ZeroMatrix2f();
    friend Matrix2f TranslationMatrix2f(const Vector3f& Location);
    friend Matrix2f RotationXMatrix2f(float Angle);
    friend Matrix2f RotationYMatrix2f(float Angle);
    friend Matrix2f RotationZMatrix2f(float Angle);
    friend Matrix2f RotationYPRMatrix2f(float Yaw, float Pitch, float Roll);
    friend Matrix2f RotationAxisMatrix2f(const Vector3f& axis, float Angle);

    friend Matrix2f ScaleMatrix2f(float X, float Y, float Z);
    friend Matrix2f GenRotation2f(const Vector3f& x, const Vector3f& y, const Vector3f& z);
    friend Matrix2f ViewMatrix2f(const Vector3f& LookAt, const Vector3f& Viewer, const Vector3f& Up);
    friend Matrix2f QuadricMatrix2f(float a, float b, float c, float d, float e,
                    	        float f, float g, float h, float j, float k);
    friend Matrix2f MirrorX2f();
    friend Matrix2f MirrorY2f();
    friend Matrix2f MirrorZ2f();
    friend Matrix2f RotationOnly2f(const Matrix2f& x);

    // Binary operators
    friend Matrix2f operator+ (const Matrix2f& A, const Matrix2f& B);
    friend Matrix2f operator- (const Matrix2f& A, const Matrix2f& B);
    friend Matrix2f operator* (const Matrix2f& A, float B);
    friend Matrix2f operator* (const Matrix2f& A, const Matrix2f& B);
    friend class Vector3f operator* (const Vector3f& v, const Matrix2f& M);
    friend class Vector3f operator* (const Matrix2f& M, const Vector3f& v); 
    friend class Vector3f RotateOnly(const Vector3f& v, const Matrix2f& M);
    friend class Vector3f RotateOnly(const Matrix2f& M, const Vector3f& v)
    {
    	return RotateOnly(v, M);
    }

    // Overloaded output operator.
    //friend ostream& operator<< (ostream& s, const Matrix2f& M);
};

#endif

