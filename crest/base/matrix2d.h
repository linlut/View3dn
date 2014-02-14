// ===================================================================
// matrix2d.h
// ===================================================================

#ifndef __MATRIX2D_89262632626326__

#define __MATRIX2D_89262632626326__


#include <crest/base/sysconf.h>
#include <crest/base/vectorall.h>


// Forward-declare some other classes

class Matrix2d 
{

public:
    double x[4][4];

    Matrix2d(void) { }

    ~Matrix2d() { }

	// determinant
	double det4x4(void) const;

	void Adjoint(void);

    // Assignment operators
    Matrix2d& operator+= (const Matrix2d& A); 	// add-to
    Matrix2d& operator-= (const Matrix2d& A); 	// subtract-from
    Matrix2d& operator*= (const Matrix2d& A); 	// multiply by Matrix2d
    Matrix2d& operator*= (double A); 		    // scale by scalar

    // Fundamental operations
    void Invert();				                // Invert the Matrix2d
    void Transpose();				            // Transpose the Matrix2d
    friend Matrix2d Invert(const Matrix2d& M);	// Invert a given Matrix2d
    friend Matrix2d Transpose(const Matrix2d& M);// Transpose a given Matrix2d

    // Create various types of Matrix2d.
    friend Matrix2d IdentityMatrix2d();
    friend Matrix2d ZeroMatrix2d();
    friend Matrix2d TranslationMatrix(const Vector3d& Location);
    friend Matrix2d RotationXMatrix2d(double Angle);
    friend Matrix2d RotationYMatrix2d(double Angle);
    friend Matrix2d RotationZMatrix2d(double Angle);
    friend Matrix2d RotationYPRMatrix2d(double Yaw, double Pitch, double Roll);
    friend Matrix2d RotationAxisMatrix(const Vector3d& axis, double Angle);

    friend Matrix2d ScaleMatrix2d(double X, double Y, double Z);
    friend Matrix2d GenRotation(const Vector3d& x, const Vector3d& y, const Vector3d& z);
    friend Matrix2d ViewMatrix(const Vector3d& LookAt, const Vector3d& Viewer, const Vector3d& Up);
    friend Matrix2d QuadricMatrix2d(double a, double b, double c, double d, double e,
                    	        double f, double g, double h, double j, double k);
    friend Matrix2d MirrorX2d();
    friend Matrix2d MirrorY2d();
    friend Matrix2d MirrorZ2d();
    friend Matrix2d RotationOnly(const Matrix2d& x);

    // Binary operators
    friend Matrix2d operator+ (const Matrix2d& A, const Matrix2d& B);
    friend Matrix2d operator- (const Matrix2d& A, const Matrix2d& B);
    friend Matrix2d operator* (const Matrix2d& A, double B);
    friend Matrix2d operator* (const Matrix2d& A, const Matrix2d& B);
    friend class Vector3d operator* (const Vector3d& v, const Matrix2d& M);
    friend class Vector3d operator* (const Matrix2d& M, const Vector3d& v); 
    friend class Vector3d RotateOnly(const Vector3d& v, const Matrix2d& M);

    // Overloaded output operator.
    //friend ostream& operator<< (ostream& s, const Matrix2d& M);
};


// a 2x2 determinant
inline double 
det2x2(const double& a, const double& b, const double& c, const double& d)
{
	return a * d - b * c;
}

// A 3x3 determinant
inline double 
det3x3(const double& a1, const double& a2, const double& a3,
	   const double& b1, const double& b2, const double& b3,
	   const double& c1, const double& c2, const double& c3 )
{
	return    a1 * det2x2( b2, b3, c2, c3 )
       		- b1 * det2x2( a2, a3, c2, c3 )
       		+ c1 * det2x2( a2, a3, b2, b3 );
}


#endif

