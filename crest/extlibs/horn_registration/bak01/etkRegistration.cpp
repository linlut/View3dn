//===========================================================================

/*! \file etkRegistration.cpp
    \brief 3D-3D point registration using the Horn method.
 */

#include <sysconf.h> 
#include <vectorall.h>
#include <matrix3x3.h>
#include "etkPlatform.hpp"

#ifdef VCC_COMPILER
	#include <memory.h>
#else
	#include <string.h>
#endif
#include <math.h>

#ifndef NULL
	#define NULL (0L)
#endif

#include "etkErrorAPI.hpp"
#include "etkQuartic.hpp"
#include "etkQuaternion.hpp"
#include "etkRegistration.hpp"

//---------------------------------------------------------------------------
// PRIVATE: determinant of a 3x3 matrix
// ----------------------------------------------------------------------------

static inline 
double etkDet33 (const double& a1, const double& b1, const double& c1,
                 const double& a2, const double& b2, const double& c2,
                 const double& a3, const double& b3, const double& c3)
{
  return a1*b2*c3 + b1*c2*a3 + c1*a2*b3 - a3*b2*c1 - b3*c2*a1 - c3*a2*b1;
}

//rot is a rotaion matrix, column major returned from the ETK package
//t is the vector to be rotated, t is column vector
inline Vector3d matMultVector3d(const double rot[3][3], const Vector3d & t)
{
	Vector3d r;
	r.x = rot[0][0]*t.x + rot[0][1]*t.y + rot[0][2]*t.z;
    r.y = rot[1][0]*t.x + rot[1][1]*t.y + rot[1][2]*t.z;
    r.z = rot[2][0]*t.x + rot[2][1]*t.y + rot[2][2]*t.z;
	return r;
}

inline Vector3d vector3dMultMat(const Vector3d & t, const double rot[3][3])
{
	Vector3d r;
	r.x = rot[0][0]*t.x + rot[1][0]*t.y + rot[2][0]*t.z;
    r.y = rot[0][1]*t.x + rot[1][1]*t.y + rot[2][1]*t.z;
    r.z = rot[0][2]*t.x + rot[1][2]*t.y + rot[2][2]*t.z;
	return r;
}

typedef double mat33[3][3];
inline Vector3d vector3dMultMat(const Vector3d & t, const double p[9])
{
	mat33* rot= (mat33*) &p[0];
	return vector3dMultMat(t, *rot);	
}

//---------------------------------------------------------------------------
// PRIVATE: comatrix of a 4x4 matrix
// ----------------------------------------------------------------------------

static inline void 
etkComatrix44 (double &a1, double &b1, double &c1, double &d1,
               double &a2, double &b2, double &c2, double &d2,
               double &a3, double &b3, double &c3, double &d3,
               double &a4, double &b4, double &c4, double &d4)
{
  double coffa1, coffa2, coffa3, coffa4,
         coffb1, coffb2, coffb3, coffb4,
         coffc1, coffc2, coffc3, coffc4,
         coffd1, coffd2, coffd3, coffd4;

  coffa1 = etkDet33 (b2, c2, d2,
                     b3, c3, d3,
                     b4, c4, d4);
  coffb1 = -etkDet33 (a2, c2, d2,
                      a3, c3, d3,
                      a4, c4, d4);
  coffc1 = etkDet33 (a2, b2, d2,
                     a3, b3, d3,
                     a4, b4, d4);
  coffd1 = -etkDet33 (a2, b2, c2,
                      a3, b3, c3,
                      a4, b4, c4);

  coffa2 = -etkDet33 (b1, c1, d1,
                      b3, c3, d3,
                      b4, c4, d4);
  coffb2 = etkDet33 (a1, c1, d1,
                     a3, c3, d3,
                     a4, c4, d4);
  coffc2 = -etkDet33 (a1, b1, d1,
                      a3, b3, d3,
                      a4, b4, d4);
  coffd2 = etkDet33 (a1, b1, c1,
                     a3, b3, c3,
                     a4, b4, c4);

  coffa3 = etkDet33 (b1, c1, d1,
                     b2, c2, d2,
                     b4, c4, d4);
  coffb3 = -etkDet33 (a1, c1, d1,
                      a2, c2, d2,
                      a4, c4, d4);
  coffc3 = etkDet33 (a1, b1, d1,
                     a2, b2, d2,
                     a4, b4, d4);
  coffd3 = -etkDet33 (a1, b1, c1,
                      a2, b2, c2,
                      a4, b4, c4);

  coffa4 = -etkDet33 (b1, c1, d1,
                      b2, c2, d2,
                      b3, c3, d3);
  coffb4 = etkDet33 (a1, c1, d1,
                     a2, c2, d2,
                     a3, c3, d3);
  coffc4 = -etkDet33 (a1, b1, d1,
                      a2, b2, d2,
                      a3, b3, d3);
  coffd4 = etkDet33 (a1, b1, c1,
                     a2, b2, c2,
                     a3, b3, c3);

  a1 = coffa1;  a2 = coffa2;  a3 = coffa3;  a4 = coffa4;
  b1 = coffb1;  b2 = coffb2;  b3 = coffb3;  b4 = coffb4;
  c1 = coffc1;  c2 = coffc2;  c3 = coffc3;  c4 = coffc4;
  d1 = coffd1;  d2 = coffd2;  d3 = coffd3;  d4 = coffd4;

}

// ----------------------------------------------------------------------------
// PRIVATE: eigen value decomposition of a 4x4 matrix
// ----------------------------------------------------------------------------

static inline void 
etkRegisterMatrix (etkQuaternion* pQuat, 
				   double dN11, double dN12, double dN13, double dN14,
                   double dN21, double dN22, double dN23, double dN24,
                   double dN31, double dN32, double dN33, double dN34,
                   double dN41, double dN42, double dN43, double dN44)
{
  // Determinant of N - y*I
  double dByyy, dCyy, dDy, dE;

  int i;

  // Ay^4 + By^3 + Cy^2 + Dy + E = 0;

  // ---- First row of the determinant ----

  // 1.1
  dByyy = - dN11 - dN22 - dN33 - dN44;
  dCyy	=   dN11*dN22 + dN11*dN33 + dN11*dN44 + dN22*dN33 + dN22*dN44 + dN33*dN44;
  dDy	= - dN11*dN22*dN33 - dN11*dN22*dN44 - dN11*dN33*dN44 - dN22*dN33*dN44;
  dE	=   dN11*dN22*dN33*dN44;
  // 1.2
  dDy  -=	dN23*dN34*dN42;
  dE   +=	dN11*dN23*dN34*dN42;
  // 1.3
  dDy  -=	dN24*dN32*dN43;
  dE   +=	dN11*dN24*dN32*dN43;
  // 1.4
  dCyy -=   dN34*dN43;
  dDy  +=	(dN11 + dN22)*dN34*dN43;
  dE   -=	dN11*dN22*dN34*dN43;
  // 1.5
  dCyy -=	dN23*dN32;
  dDy  +=	(dN11 + dN44)*dN23*dN32;
  dE   -=	dN11*dN23*dN32*dN44;
  // 1.6
  dCyy -=	dN24*dN42;
  dDy  +=	(dN11 + dN33)*dN24*dN42;
  dE   -=	dN11*dN24*dN33*dN42;

  // ---- Second row of the determinant ----

  // 2.1
  dCyy -= dN21*dN12;
  dDy += dN21*dN12*(dN33 + dN44);
  dE -= dN21*dN12*dN33*dN44;
  // 2.2
  dE -= dN21*dN13*dN34*dN42;
  // 2.3
  dE -= dN21*dN14*dN32*dN43;
  // 2.4
  dE += dN21*dN12*dN34*dN43;
  // 2.5
  dDy -= dN21*dN13*dN32;
  dE += dN21*dN13*dN32*dN44;
  // 2.6
  dDy -= dN21*dN14*dN42;
  dE += dN21*dN14*dN33*dN42;

  // ---- Third row of the determinant ----

  // 3.1
  dDy -= dN31*dN12*dN23;
  dE += dN31*dN12*dN23*dN44;
  // 3.2
  dE += dN31*dN13*dN24*dN42;
  // 3.3
  dDy -= dN31*dN14*dN43;
  dE += dN31*dN14*dN22*dN43;
  // 3.4
  dE -= dN31*dN12*dN24*dN43;
  // 3.5
  dCyy -= dN31*dN13;
  dDy += dN31*dN13*(dN22 + dN44);
  dE -= dN31*dN13*dN22*dN44;
  // 3.6
  dE -= dN31*dN14*dN23*dN42;

  // ---- Forth row of the determinant ----

  // 4.1
  dE -= dN41*dN12*dN23*dN34;
  // 4.2
  dE -= dN41*dN13*dN24*dN32;
  // 4.3
  dCyy -= dN41*dN14;
  dDy += dN41*dN14*(dN22 + dN33);
  dE -= dN41*dN14*dN22*dN33;
  // 4.4
  dDy -= dN41*dN12*dN24;
  dE += dN41*dN12*dN24*dN33;
  // 4.5
  dDy -= dN41*dN13*dN34;
  dE += dN41*dN13*dN34*dN22;
  // 4.6
  dE += dN41*dN14*dN23*dN32;

  // Solving the quartic equation to obtain the eigenvalues
  double adRoots [4]={0,0,0,0};
  const int iNbRoots = etkQuartic (dByyy, dCyy, dDy, dE, adRoots);
/*
  SymmetricMatrix A(4);
  A(1,1) = dN11, A(1,2)=dN12, A(1,3)=dN13, A(1,4)=dN14;
  A(2,2) = dN22, A(2,3)=dN23, A(2,4)=dN24;
  A(3,3) = dN33, A(3,4)=dN34;
  A(4,4) = dN44;
  DiagonalMatrix D(4);
  EigenValues(A,D);
  double eng1=D(1,1);
  double eng2=D(2,2);
  double eng3=D(3,3);
  double eng4=D(4,4);
*/

  // Finding the  argest eigenvalue
  double dMaxEigenValue = adRoots[0];
  for (i=1; i<iNbRoots; i++){
    if (adRoots[i] > dMaxEigenValue)
      dMaxEigenValue = adRoots[i];
  }

  double dN11bis = dN11 - dMaxEigenValue;
  double dN22bis = dN22 - dMaxEigenValue;
  double dN33bis = dN33 - dMaxEigenValue;
  double dN44bis = dN44 - dMaxEigenValue;

  etkComatrix44 (dN11bis, dN12, dN13, dN14,
                 dN21, dN22bis, dN23, dN24,
                 dN31, dN32, dN33bis, dN34,
                 dN41, dN42, dN43, dN44bis);

  etkCreateQuaternion (pQuat,  
	    dN11bis+dN21+dN31+dN41, dN12+dN22bis+dN32+dN42,
        dN13+dN23+dN33bis+dN43, dN14+dN24+dN34+dN44bis);
  etkNormalizeQuaterion (pQuat);
}


// ----------------------------------------------------------------------------
// Register the two point sets.
// - Rotation matrix is stored in p33Rot,
// - Translation vector is stored in pTrans
// - Return error matrix
// ----------------------------------------------------------------------------

#define ADD3D(d,s)         {d[0] += s[0]; d[1] += s[1]; d[2] += s[2];};
#define SUB3D(d,s)         {d[0] -= s[0]; d[1] -= s[1]; d[2] -= s[2];};
#define MULTSCAL3D(d,scal) {d[0] *= (scal); d[1] *= (scal); d[2] *= (scal);};
#define MAT33(a,b)         (matRot33.block [(a) + ((b<<1)+b)])

inline void weightedSum(Vector3d* p, double *w, const int n, Vector3d& sum)
{
	sum = p[0]*w[0];
	for (int i=1; i<n; i++) sum+=p[i]*w[i];
}


double etkRegistration::_etkRegister(etkQuaternion* pQuat, double* radRot33, double* radTrans3, const bool getRMS)
{
	int u;
	etkRegistration *pRegistration = this;
	Vector3d adModelCentroid, adSensorCentroid, *v1, *v2;
	const double scale = 1; 

	// 1. Find centroid
	weightedSum(adModelPoints, weights, uNbPoints, adModelCentroid);
	weightedSum(adSensorPoints, weights, uNbPoints, adSensorCentroid);

	// 2. Recenter points;
	for (u = 0; u < uNbPoints; u++){
		adModelPoints[u] -= adModelCentroid;
		adSensorPoints[u] -= adSensorCentroid;
	}

	// 3. Compute momentum
	double dSxx, dSxy, dSxz, dSyx, dSyy, dSyz, dSzx, dSzy, dSzz;
	dSxx=dSxy=dSxz=dSyx=dSyy=dSyz=dSzx=dSzy=dSzz=0;
	for (u = 0; u < uNbPoints; u++){
		v1 = &adModelPoints[u];
		v2 = &adSensorPoints[u];
		const double w = weights[u];
		const double p0=v1->x, p1=v1->y, p2=v1->z;
		const double q0=v2->x*w, q1=v2->y*w, q2=v2->z*w;	
		dSxx += p0 * q0;
		dSxy += p0 * q1;
		dSxz += p0 * q2;
		dSyx += p1 * q0;
		dSyy += p1 * q1;
		dSyz += p1 * q2;
		dSzx += p2 * q0;
		dSzy += p2 * q1;
		dSzz += p2 * q2;
	}

	// N Symmetric Matrix
	double dN11 = dSxx + dSyy + dSzz;
	double dN12 = dSyz - dSzy;
	double dN13 = dSzx - dSxz;
	double dN14 = dSxy - dSyx;
	double dN21 = dN12;
	double dN22 = dSxx - dSyy - dSzz;
	double dN23 = dSxy + dSyx;
	double dN24 = dSzx + dSxz;
	double dN31 = dN13;
	double dN32 = dN23;
	double dN33 = -dSxx + dSyy - dSzz;
	double dN34 = dSyz + dSzy;
	double dN41 = dN14;
	double dN42 = dN24;
	double dN43 = dN34;
	double dN44 = -dSxx - dSyy + dSzz;

	// compute rotation
	etkRegisterMatrix (pQuat, 
					dN11, dN12, dN13, dN14, dN21, dN22, dN23, dN24,
					dN31, dN32, dN33, dN34, dN41, dN42, dN43, dN44);
	matrix matRot33  = {3, 3, radRot33};
	etkQuaterionToMatrix33_v2(pQuat, &matRot33);	//here, vector is column vector to mult the matrix
													//therefore, we use y=Ax
    // compute translation
	/*
	matrix matTrans3 = {3, 1, radTrans3};
	matrix matModelCentroid = {3, 1, &adModelCentroid.x};
	mmult (&matRot33, &matModelCentroid, &matTrans3);
	double xx, yy, zz;
	u=0; xx = adSensorCentroid [u] - matTrans3.block [u];
	u=1; yy = adSensorCentroid [u] - matTrans3.block [u];
	u=2; zz = adSensorCentroid [u] - matTrans3.block [u];
	*/
	double3x3 *pmat = (double3x3*)&matRot33.block[0];
	Vector3d *trans = (Vector3d*)&radTrans3[0];
	*trans = adSensorCentroid - (*pmat)*adModelCentroid;

	/// Compute error
	double dError = 0.0;
	if (getRMS){
		for (u = 0; u < pRegistration->uNbPoints; u++){
			Vector3d lr = (*pmat)*pRegistration->adSensorPoints[u];
			dError += Distance2(lr, pRegistration->adModelPoints[u]);
		}
	}

	return dError;
}

// ----------------------------------------------------------------------------

