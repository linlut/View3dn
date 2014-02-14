// ===================================================================
// Matrix2d.cpp
// ===================================================================

#include <math.h>
#include "matrix2d.h"
#include "matrix2f.h"



void Matrix2d::Dump(void)
{
    for (int i = 0; i < 4; i++){
		const double *p = this->x[i];
		printf("%d: %lf %lf %lf %lf\n", i, p[0], p[1], p[2], p[3]);
	}
}


// Invert the Matrix2d using Gaussian elimination.  Not efficient,
// nor does it check for singular matrices.

// determinant
double Matrix2d::det4x4(void) const
{
    double ans;
    double a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

    /* assign to individual variable names to aid selecting */
	/*  correct elements */
	a1 = x[0][0]; b1 = x[0][1]; 
	c1 = x[0][2]; d1 = x[0][3];
	a2 = x[1][0]; b2 = x[1][1]; 
	c2 = x[1][2]; d2 = x[1][3];

	a3 = x[2][0]; b3 = x[2][1]; 
	c3 = x[2][2]; d3 = x[2][3];
	a4 = x[3][0]; b4 = x[3][1]; 
	c4 = x[3][2]; d4 = x[3][3];

    ans = a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
        - b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
        + c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
        - d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
    return ans;
}

/* 
 *   adjoint( original_matrix, inverse_matrix )
 *     calculate the adjoint of a 4x4 matrix
 *
 *      Let  a   denote the minor determinant of matrix A obtained by
 *           ij
 *
 *      deleting the ith row and jth column from A.
 *
 *                    i+j
 *     Let  b   = (-1)    a
 *          ij            ji
 *
 *    The matrix B = (b  ) is the adjoint of A
 *                     ij
 */
/*
Matrix Adjoint(const Matrix& M)
{
    Matrix m(M);
    m.Transpose();
    return m;
}
*/
void Matrix2d::Adjoint(void)
{
    double a1, a2, a3, a4, b1, b2, b3, b4;
    double c1, c2, c3, c4, d1, d2, d3, d4;

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


//inverse( in, out ) Matrix4 *in, *out;
void Matrix2d::Invert()
{
	const double SMALL_NUMBER=1.e-12;
    const double det = det4x4();
    if ( fabs( det ) < SMALL_NUMBER) {
        cerr<<"Non-singular matrix, no inverse!\n";
        exit(1);
    }

    Adjoint();
	const double det1 = 1.0/det;
    for (int i=0; i<4; i++)
        for(int j=0; j<4; j++) 
			x[i][j] *= det1;
}


/*
void Matrix2d::Invert()
{
    int i, j, k;
    Matrix2d Out = IdentityMatrix2d();

    for (i = 0; i < 4; i++) {
	if (x[i][i] != 1.0) {
	    double divby = x[i][i];
	    for (j = 0; j < 4; j++) {
		Out.x[i][j] /= divby;
		x[i][j] /= divby;
	    }
	}
	for (j = 0; j < 4; j++) {
	    if (j != i) {
		if (x[j][i] != 0.0) {
		    double mulby = x[j][i];
		      for (k = 0; k < 4; k++) {
			  x[j][k] -= mulby * x[i][k];
			  Out.x[j][k] -= mulby * Out.x[i][k];
		      }
		}
	    }
	}
    }
    *this = Out;
}
*/

// Invert the given Matrix2d using the above inversion routine.
Matrix2d Invert(const Matrix2d& M)
{
    Matrix2d InvertMe = M;
    InvertMe.Invert();
    return InvertMe;
}

// Transpose the Matrix2d.
void Matrix2d::Transpose()
{
	for (int i = 0; i < 4; i++){
		for (int j = i; j < 4; j++){
			if (i != j) {
				const double temp = x[i][j];
				x[i][j] = x[j][i];
				x[j][i] = temp;
			}
		}
	}
}

// Transpose the given Matrix2d using the transpose routine above.
Matrix2d Transpose(const Matrix2d& M)
{
    Matrix2d TransposeMe = M;
    TransposeMe.Transpose();
    return TransposeMe;
}

// Construct an identity Matrix2d.
Matrix2d IdentityMatrix2d()
{
    Matrix2d M;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    M.x[i][j] = (i == j) ? 1.0 : 0.0;
    return M;
}

// Construct a zero Matrix2d.
Matrix2d ZeroMatrix2d()
{
    Matrix2d M;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M.x[i][j] = 0;
    return M;
}

// Construct a translation Matrix2d given the location to translate to.
Matrix2d TranslationMatrix(const Vector3d& Location)
{
    Matrix2d M = IdentityMatrix2d();
    M.x[3][0] = Location.x;
    M.x[3][1] = Location.y;
    M.x[3][2] = Location.z;
    return M;
}

// Construct a rotation Matrix2d.  Rotates Angle radians about the
// X axis.
Matrix2d RotationXMatrix2d(double Angle)
{
    Matrix2d M = IdentityMatrix2d();
    double Cosine = cos(Angle);
    double Sine = sin(Angle);
    M.x[1][1] = Cosine;
    M.x[2][1] = -Sine;
    M.x[1][2] = Sine;
    M.x[2][2] = Cosine;
    return M;
}

// Construct a rotation Matrix2d.  Rotates Angle radians about the
// Y axis.
Matrix2d RotationYMatrix2d(double Angle)
{
    Matrix2d M = IdentityMatrix2d();
    double Cosine = cos(Angle);
    double Sine = sin(Angle);
    M.x[0][0] = Cosine;
    M.x[2][0] = -Sine;
    M.x[0][2] = Sine;
    M.x[2][2] = Cosine;
    return M;
}

// Construct a rotation Matrix2d.  Rotates Angle radians about the
// Z axis.
Matrix2d RotationZMatrix2d(double Angle)
{
    Matrix2d M = IdentityMatrix2d();
    double Cosine = cos(Angle);
    double Sine = sin(Angle);
    M.x[0][0] = Cosine;
    M.x[1][0] = -Sine;
    M.x[0][1] = Sine;
    M.x[1][1] = Cosine;
    return M;
}

// Construct a yaw-pitch-roll rotation Matrix2d.	Rotate Yaw
// radians about the XY axis, rotate Pitch radians in the
// plane defined by the Yaw rotation, and rotate Roll radians
// about the axis defined by the previous two angles.
Matrix2d RotationYPRMatrix2d(double Yaw, double Pitch, double Roll)
{
    Matrix2d M;
    double ch = cos(Yaw);
    double sh = sin(Yaw);
    double cp = cos(Pitch);
    double sp = sin(Pitch);
    double cr = cos(Roll);
    double sr = sin(Roll);

    M.x[0][0] = ch*cr + sh*sp*sr;
    M.x[1][0] = -ch*sr + sh*sp*cr;
    M.x[2][0] = sh*cp;
    M.x[0][1] = sr*cp;
    M.x[1][1] = cr*cp;
    M.x[2][1] = -sp;
    M.x[0][2] = -sh*cr - ch*sp*sr;
    M.x[1][2] = sr*sh + ch*sp*cr;
    M.x[2][2] = ch*cp;
    for (int i = 0; i < 4; i++)
	M.x[3][i] = M.x[i][3] = 0;
    M.x[3][3] = 1;

    return M;
}

// Construct a rotation of a given angle about a given axis.
// Derived from Eric Haines's SPD (Standard Procedural 
// Database).
Matrix2d
RotationAxisMatrix(const Vector3d& axis, double angle)
{
    Matrix2d M;
    double cosine = cos(angle);
    double sine = sin(angle);
    double one_minus_cosine = 1 - cosine;

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

    M.x[3][0] = 0;
    M.x[3][1] = 0;
    M.x[3][2] = 0;
    M.x[3][3] = 1;

    return M;
}
    

// Construct a scale Matrix2d given the X, Y, and Z parameters
// to scale by.  To scale uniformly, let X==Y==Z.
Matrix2d ScaleMatrix2d(double X, double Y, double Z)
{
    Matrix2d M = IdentityMatrix2d();

    M.x[0][0] = X;
    M.x[1][1] = Y;
    M.x[2][2] = Z;

    return M;
}

// Construct a rotation Matrix2d that makes the x, y, z axes
// correspond to the vectors given.
Matrix2d GenRotation(const Vector3d& x, const Vector3d& y, const Vector3d& z)
{
    Matrix2d M = IdentityMatrix2d();

    M.x[0][0] = x.x;
    M.x[1][0] = x.y;
    M.x[2][0] = x.z;
    M.x[0][1] = y.x;
    M.x[1][1] = y.y;
    M.x[2][1] = y.z;
    M.x[0][2] = z.x;
    M.x[1][2] = z.y;
    M.x[2][2] = z.z;

    return M;
}

// Construct a view Matrix2d to rotate and translate the world
// given that the viewer is located at Viewer, looking at
// LookAt, and the up vector is UpL.  The transformation
// changes things around so the viewer is at the origin and
// looking down the -Z axis.
Matrix2d ViewMatrix(const Vector3d& LookAt, const Vector3d& Viewer, const Vector3d& UpL)
{
    Matrix2d M = IdentityMatrix2d();
    Vector3d U, V, N;
    Vector3d Up = Normalize(UpL);

    N = Normalize(Viewer - LookAt);

    V = Up - LookAt;
    V -= N * DotProd(V, N);
    V = Normalize(V);
//    V = Normalize(Up - N * DotProd(Up, N));

    U = CrossProd(V, N);

    M.x[0][0] = U.x;
    M.x[1][0] = U.y;
    M.x[2][0] = U.z;
    M.x[0][1] = V.x;
    M.x[1][1] = V.y;
    M.x[2][1] = V.z;
    M.x[0][2] = N.x;
    M.x[1][2] = N.y;
    M.x[2][2] = N.z;

    M.x[3][0] = -DotProd(U, Viewer);
    M.x[3][1] = -DotProd(V, Viewer);
    M.x[3][2] = -DotProd(N, Viewer);

    return M;
}

// Construct a quadric Matrix2d.	After Foley et al. pp. 528-529.
Matrix2d
QuadricMatrix2d(double a, double b, double c, double d, double e,double f, double g, double h, double j, double k)
{
    Matrix2d M;

    M.x[0][0] = a;  M.x[0][1] = d;  M.x[0][2] = f;  M.x[0][3] = g;
    M.x[1][0] = d;  M.x[1][1] = b;  M.x[1][2] = e;  M.x[1][3] = h;
    M.x[2][0] = f;  M.x[2][1] = e;  M.x[2][2] = c;  M.x[2][3] = j;
    M.x[3][0] = g;  M.x[3][1] = h;  M.x[3][2] = j;  M.x[3][3] = k;

    return M;
}

// Construct various "mirror" matrices, which flip coordinate
// signs in the various axes specified.
Matrix2d MirrorX2d()
{
    Matrix2d M = IdentityMatrix2d();
    M.x[0][0] = -1;
    return M;
}

Matrix2d MirrorY2d()
{
    Matrix2d M = IdentityMatrix2d();
    M.x[1][1] = -1;
    return M;
}

Matrix2d MirrorZ2d()
{
    Matrix2d M = IdentityMatrix2d();
    M.x[2][2] = -1;
    return M;
}

Matrix2d RotationOnly(const Matrix2d& x)
{
    Matrix2d M = x;
    M.x[3][0] = M.x[3][1] = M.x[3][2] = 0;
    return M;
}

// Add corresponding elements of the two matrices.
Matrix2d& Matrix2d::operator+= (const Matrix2d& A)
{
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    x[i][j] += A.x[i][j];
    return *this;
}

// Subtract corresponding elements of the matrices.
Matrix2d&
Matrix2d::operator-= (const Matrix2d& A)
{
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    x[i][j] -= A.x[i][j];
    return *this;
}

// Scale each element of the Matrix2d by A.
Matrix2d&
Matrix2d::operator*= (double A)
{
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    x[i][j] *= A;
    return *this;
}

// Multiply two matrices.
Matrix2d&
Matrix2d::operator*= (const Matrix2d& A)
{
    Matrix2d ret = *this;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++) {
	    double subt = 0;
	    for (int k = 0; k < 4; k++)
		subt += ret.x[i][k] * A.x[k][j];
	    x[i][j] = subt;
	}
    return *this;
}

// Add corresponding elements of the matrices.
Matrix2d
operator+ (const Matrix2d& A, const Matrix2d& B)
{
    Matrix2d ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    ret.x[i][j] = A.x[i][j] + B.x[i][j];
    return ret;
}

// Subtract corresponding elements of the matrices.
Matrix2d
operator- (const Matrix2d& A, const Matrix2d& B)
{
    Matrix2d ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    ret.x[i][j] = A.x[i][j] - B.x[i][j];
    return ret;
}

// Multiply matrices.
Matrix2d
operator* (const Matrix2d& A, const Matrix2d& B)
{
    Matrix2d ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++) {
	    double subt = 0;
	    for (int k = 0; k < 4; k++)
		subt += A.x[i][k] * B.x[k][j];
	    ret.x[i][j] = subt;
	}
    return ret;
}

// Transform a vector by a Matrix2d.
Vector3d operator* (const Vector3d& v, const Matrix2d& M)
{
    Vector3d ret;
    ret.x = v.x * M.x[0][0] + v.y * M.x[1][0] + v.z * M.x[2][0] + M.x[3][0];
    ret.y = v.x * M.x[0][1] + v.y * M.x[1][1] + v.z * M.x[2][1] + M.x[3][1];
    ret.z = v.x * M.x[0][2] + v.y * M.x[1][2] + v.z * M.x[2][2] + M.x[3][2];
    double denom = M.x[0][3] + M.x[1][3] + M.x[2][3] + M.x[3][3];
    if (denom != 1.0)
	ret /= denom;
    return ret;
}


Vector3d operator* (const Matrix2d& M, const Vector3d& v)
{ 
	return v*M; 
}


// Apply the rotation portion of a Matrix2d to a vector.
Vector3d
RotateOnly(const Vector3d& v, const Matrix2d& M)
{
    Vector3d ret;

    ret.x = v.x * M.x[0][0] + v.y * M.x[1][0] + v.z * M.x[2][0];
    ret.y = v.x * M.x[0][1] + v.y * M.x[1][1] + v.z * M.x[2][1];
    ret.z = v.x * M.x[0][2] + v.y * M.x[1][2] + v.z * M.x[2][2];
    //double denom = M.x[0][3] + M.x[1][3] + M.x[2][3] + M.x[3][3];
    //if (denom != 1.0)
	//ret /= denom;
    return ret;
}

// Scale each element of the Matrix2d by B.
Matrix2d
operator* (const Matrix2d& A, double B)
{
    Matrix2d ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    ret.x[i][j] = A.x[i][j] * B;
    return ret;
}

// Overloaded << for C++-style output.
/*
ostream& operator<< (ostream& s, const Matrix2d& M)
{
    for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 4; j++)
	    s << setprecision(2) << setw(10) << M.x[i][j];
	s << '\n';
    }
    return s;
}
*/


// Rotate a direction vector...
Vector3d
PlaneRotate(const Matrix2d& tform, const Vector3d& p)
{
    // I sure hope that Matrix2d is invertible...
    Matrix2d use = Transpose(Invert(tform));

    return RotateOnly(p, use);
}


//======================================================================
//======================================================================
//======================================================================

// Invert the matrix using Gaussian elimination.  Not efficient,
// nor does it check for singular matrices.
static inline void _copyDM2FM(const Matrix2d& dm, Matrix2f &fm)
{
	const double *src = &dm.x[0][0];
	float *dst=&fm.x[0][0];
	for (int i=0; i<16; i++) dst[i]=src[i];
}

static inline void _copyFM2DM(const Matrix2f& fm, Matrix2d &dm)
{
	const float *src = &fm.x[0][0];
	double *dst=&dm.x[0][0];
	for (int i=0; i<16; i++) dst[i]=src[i];
}

void Matrix2f::Invert()
{
	Matrix2d m;
	_copyFM2DM(*this, m);
	m.Invert();
	_copyDM2FM(m, *this);	
}

// Invert the given matrix using the above inversion routine.
Matrix2f Invert(const Matrix2f& M)
{
    Matrix2f InvertMe = M;
    InvertMe.Invert();
    return InvertMe;
}

// Transpose the matrix.
void Matrix2f::Transpose()
{
    for (int i = 0; i < 4; i++)
	for (int j = i; j < 4; j++)
	    if (i != j) {
		float temp = x[i][j];
		x[i][j] = x[j][i];
		x[j][i] = temp;
	    }
}

// Transpose the given matrix using the transpose routine above.
Matrix2f Transpose(const Matrix2f& M)
{
    Matrix2f TransposeMe = M;
    TransposeMe.Transpose();
    return TransposeMe;
}

// Construct an identity matrix.
Matrix2f IdentityMatrix2f()
{
    Matrix2f M;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    M.x[i][j] = (i == j) ? 1.0 : 0.0;
    return M;
}

// Construct a zero matrix.
Matrix2f ZeroMatrix2f()
{
    Matrix2f M;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M.x[i][j] = 0;
    return M;
}

// Construct a translation matrix given the location to translate to.
Matrix2f TranslationMatrix2f(const Vector3f& Location)
{
    Matrix2f M = IdentityMatrix2f();
    M.x[3][0] = Location.x;
    M.x[3][1] = Location.y;
    M.x[3][2] = Location.z;
    return M;
}

// Construct a rotation matrix.  Rotates Angle radians about the
// X axis.
Matrix2f RotationXMatrix2f(float Angle)
{
    Matrix2f M = IdentityMatrix2f();
    float Cosine = cos(Angle);
    float Sine = sin(Angle);
    M.x[1][1] = Cosine;
    M.x[2][1] = -Sine;
    M.x[1][2] = Sine;
    M.x[2][2] = Cosine;
    return M;
}

// Construct a rotation matrix.  Rotates Angle radians about the
// Y axis.
Matrix2f RotationYMatrix2f(float Angle)
{
    Matrix2f M = IdentityMatrix2f();
    float Cosine = cos(Angle);
    float Sine = sin(Angle);
    M.x[0][0] = Cosine;
    M.x[2][0] = -Sine;
    M.x[0][2] = Sine;
    M.x[2][2] = Cosine;
    return M;
}

// Construct a rotation matrix.  Rotates Angle radians about the
// Z axis.
Matrix2f RotationZMatrix2f(float Angle)
{
    Matrix2f M = IdentityMatrix2f();
    float Cosine = cos(Angle);
    float Sine = sin(Angle);
    M.x[0][0] = Cosine;
    M.x[1][0] = -Sine;
    M.x[0][1] = Sine;
    M.x[1][1] = Cosine;
    return M;
}

// Construct a yaw-pitch-roll rotation matrix.	Rotate Yaw
// radians about the XY axis, rotate Pitch radians in the
// plane defined by the Yaw rotation, and rotate Roll radians
// about the axis defined by the previous two angles.
Matrix2f RotationYPRMatrix2f(float Yaw, float Pitch, float Roll)
{
    Matrix2f M;
    float ch = cos(Yaw);
    float sh = sin(Yaw);
    float cp = cos(Pitch);
    float sp = sin(Pitch);
    float cr = cos(Roll);
    float sr = sin(Roll);

    M.x[0][0] = ch*cr + sh*sp*sr;
    M.x[1][0] = -ch*sr + sh*sp*cr;
    M.x[2][0] = sh*cp;
    M.x[0][1] = sr*cp;
    M.x[1][1] = cr*cp;
    M.x[2][1] = -sp;
    M.x[0][2] = -sh*cr - ch*sp*sr;
    M.x[1][2] = sr*sh + ch*sp*cr;
    M.x[2][2] = ch*cp;
    for (int i = 0; i < 4; i++)
	M.x[3][i] = M.x[i][3] = 0;
    M.x[3][3] = 1;

    return M;
}

// Construct a rotation of a given angle about a given axis.
// Derived from Eric Haines's SPD (Standard Procedural 
// Database).
Matrix2f RotationAxisMatrix2f(const Vector3f& axis, float angle)
{
    Matrix2f M;
    float cosine = cos(angle);
    float sine = sin(angle);
    float one_minus_cosine = 1 - cosine;

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

    M.x[3][0] = 0;
    M.x[3][1] = 0;
    M.x[3][2] = 0;
    M.x[3][3] = 1;

    return M;
}
    

// Construct a scale matrix given the X, Y, and Z parameters
// to scale by.  To scale uniformly, let X==Y==Z.
Matrix2f ScaleMatrix2f(float X, float Y, float Z)
{
    Matrix2f M = IdentityMatrix2f();

    M.x[0][0] = X;
    M.x[1][1] = Y;
    M.x[2][2] = Z;

    return M;
}

// Construct a rotation matrix that makes the x, y, z axes
// correspond to the vectors given.
Matrix2f GenRotation2f(const Vector3f& x, const Vector3f& y, const Vector3f& z)
{
    Matrix2f M = IdentityMatrix2f();

    M.x[0][0] = x.x;
    M.x[1][0] = x.y;
    M.x[2][0] = x.z;
    M.x[0][1] = y.x;
    M.x[1][1] = y.y;
    M.x[2][1] = y.z;
    M.x[0][2] = z.x;
    M.x[1][2] = z.y;
    M.x[2][2] = z.z;

    return M;
}

// Construct a view matrix to rotate and translate the world
// given that the viewer is located at Viewer, looking at
// LookAt, and the up vector is UpL.  The transformation
// changes things around so the viewer is at the origin and
// looking down the -Z axis.
Matrix2f ViewMatrix2f(const Vector3f& LookAt, const Vector3f& Viewer, const Vector3f& UpL)
{
    Vector3f ZZ = Normalize(Viewer - LookAt);
    Vector3f XX = CrossProd(UpL, ZZ); XX.Normalize();
	Vector3f YY = CrossProd(ZZ, XX); YY.Normalize();

    Matrix2f M = IdentityMatrix2f();
    M.x[0][0] = XX.x;
    M.x[1][0] = XX.y;
    M.x[2][0] = XX.z;
    M.x[0][1] = YY.x;
    M.x[1][1] = YY.y;
    M.x[2][1] = YY.z;
    M.x[0][2] = ZZ.x;
    M.x[1][2] = ZZ.y;
    M.x[2][2] = ZZ.z;
    M.x[3][0] = -DotProd(XX, Viewer);
    M.x[3][1] = -DotProd(YY, Viewer);
    M.x[3][2] = -DotProd(ZZ, Viewer);

    return M;
}

// Construct a quadric matrix.	After Foley et al. pp. 528-529.
Matrix2f QuadricMatrix2f(float a, float b, float c, float d, float e,float f, float g, float h, float j, float k)
{
    Matrix2f M;

    M.x[0][0] = a;  M.x[0][1] = d;  M.x[0][2] = f;  M.x[0][3] = g;
    M.x[1][0] = d;  M.x[1][1] = b;  M.x[1][2] = e;  M.x[1][3] = h;
    M.x[2][0] = f;  M.x[2][1] = e;  M.x[2][2] = c;  M.x[2][3] = j;
    M.x[3][0] = g;  M.x[3][1] = h;  M.x[3][2] = j;  M.x[3][3] = k;

    return M;
}

// Construct various "mirror" matrices, which flip coordinate
// signs in the various axes specified.
Matrix2f MirrorX2f()
{
    Matrix2f M = IdentityMatrix2f();
    M.x[0][0] = -1;
    return M;
}

Matrix2f MirrorY2f()
{
    Matrix2f M = IdentityMatrix2f();
    M.x[1][1] = -1;
    return M;
}

Matrix2f MirrorZ2f()
{
    Matrix2f M = IdentityMatrix2f();
    M.x[2][2] = -1;
    return M;
}

Matrix2f RotationOnly(const Matrix2f& x)
{
    Matrix2f M = x;
    M.x[3][0] = M.x[3][1] = M.x[3][2] = 0;
    return M;
}

// Add corresponding elements of the two matrices.
Matrix2f& Matrix2f::operator+= (const Matrix2f& A)
{
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    x[i][j] += A.x[i][j];
    return *this;
}

// Subtract corresponding elements of the matrices.
Matrix2f&
Matrix2f::operator-= (const Matrix2f& A)
{
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    x[i][j] -= A.x[i][j];
    return *this;
}

// Scale each element of the matrix by A.
Matrix2f&
Matrix2f::operator*= (float A)
{
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    x[i][j] *= A;
    return *this;
}

// Multiply two matrices.
Matrix2f&
Matrix2f::operator*= (const Matrix2f& A)
{
    Matrix2f ret = *this;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++) {
	    float subt = 0;
	    for (int k = 0; k < 4; k++)
		subt += ret.x[i][k] * A.x[k][j];
	    x[i][j] = subt;
	}
    return *this;
}

// Add corresponding elements of the matrices.
Matrix2f
operator+ (const Matrix2f& A, const Matrix2f& B)
{
    Matrix2f ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    ret.x[i][j] = A.x[i][j] + B.x[i][j];
    return ret;
}

// Subtract corresponding elements of the matrices.
Matrix2f
operator- (const Matrix2f& A, const Matrix2f& B)
{
    Matrix2f ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    ret.x[i][j] = A.x[i][j] - B.x[i][j];
    return ret;
}

// Multiply matrices.
Matrix2f
operator* (const Matrix2f& A, const Matrix2f& B)
{
    Matrix2f ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++) {
	    float subt = 0;
	    for (int k = 0; k < 4; k++)
		subt += A.x[i][k] * B.x[k][j];
	    ret.x[i][j] = subt;
	}
    return ret;
}

// Transform a vector by a matrix.
Vector3f operator* (const Vector3f& v, const Matrix2f& M)
{
    Vector3f ret;
    ret.x = v.x * M.x[0][0] + v.y * M.x[1][0] + v.z * M.x[2][0] + M.x[3][0];
    ret.y = v.x * M.x[0][1] + v.y * M.x[1][1] + v.z * M.x[2][1] + M.x[3][1];
    ret.z = v.x * M.x[0][2] + v.y * M.x[1][2] + v.z * M.x[2][2] + M.x[3][2];
    float denom = M.x[0][3] + M.x[1][3] + M.x[2][3] + M.x[3][3];
    if (denom != 1.0)
	ret /= denom;
    return ret;
}


Vector3f operator* (const Matrix2f& M, const Vector3f& v)
{ 
	return v*M; 
}


// Apply the rotation portion of a matrix to a vector.
Vector3f
RotateOnly(const Vector3f& v, const Matrix2f& M)
{
    Vector3f ret;

    ret.x = v.x * M.x[0][0] + v.y * M.x[1][0] + v.z * M.x[2][0];
    ret.y = v.x * M.x[0][1] + v.y * M.x[1][1] + v.z * M.x[2][1];
    ret.z = v.x * M.x[0][2] + v.y * M.x[1][2] + v.z * M.x[2][2];
    //float denom = M.x[0][3] + M.x[1][3] + M.x[2][3] + M.x[3][3];
    //if (denom != 1.0)
	//ret /= denom;
    return ret;
}

// Scale each element of the matrix by B.
Matrix2f
operator* (const Matrix2f& A, float B)
{
    Matrix2f ret;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    ret.x[i][j] = A.x[i][j] * B;
    return ret;
}

// Overloaded << for C++-style output.
/*
ostream& operator<< (ostream& s, const Matrix2f& M)
{
    for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 4; j++)
	    s << setprecision(2) << setw(10) << M.x[i][j];
	s << '\n';
    }
    return s;
}
*/


// Rotate a direction vector...
Vector3f
PlaneRotate(const Matrix2f& tform, const Vector3f& p)
{
    // I sure hope that matrix is invertible...
    Matrix2f use = Transpose(Invert(tform));

    return RotateOnly(p, use);
}
