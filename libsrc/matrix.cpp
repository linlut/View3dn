// ===================================================================
// _Matrix2T.cpp
// ===================================================================

#include <math.h>
#include "matrix2d.h"
#include "matrix2f.h"


// Overloaded << for C++-style output.
/*
ostream& operator<< (ostream& s, const _Matrix2T& M)
{
    for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 4; j++)
	    s << setprecision(2) << setw(10) << M.x[i][j];
		s << '\n';
    }
    return s;
}
*/

/*

// Rotate a direction vector...
Vector3f
PlaneRotate(const Matrix2f& tform, const Vector3f& p)
{
    // I sure hope that matrix is invertible...
    Matrix2f use = Transpose(Invert(tform));

    return RotateOnly(p, use);
}
*/