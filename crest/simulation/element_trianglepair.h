//FILE: element_trianglepair.h
//Using two 3D triangle element for each tet element

#ifndef _INC_TRIPAIR_ZN_20091028H
#define _INC_TRIPAIR_ZN_20091028H

#include <math.h>
#include <vector3d.h>
#include <matrix3x3.h>
#include "mechanical_property.h"

class CThinshellElementFEM;


//construct the two local coord sys. of the shell element using current nodal positions
//the aixs of the coord sys are stored in two 3x3 matrices
//static inline 
inline void getReferencePlanesForTrianglePair(
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, 
	Vector3d &facenorm0, Vector3d &facenorm1, double3x3& rot0, double3x3& rot1, double &xlen)
{
	Vector3d &X0 = *((Vector3d*)(&rot1.x[0]));
	Vector3d &Y0 = *((Vector3d*)(&rot1.x[3]));
	Vector3d &Z0 = *((Vector3d*)(&rot1.x[6]));
	Vector3d &X1 = *((Vector3d*)(&rot0.x[0]));
	Vector3d &Y1 = *((Vector3d*)(&rot0.x[3]));
	Vector3d &Z1 = *((Vector3d*)(&rot0.x[6]));
	//set X axis as the vector q1
	xlen = Magnitude(q1);
	X0 = X1 = q1/xlen; 
	//compute Z axis
	Y0 = q2; Z0 = CrossProd(X0, Y0); Z0.normalize();
	Y1 = q3; Z1 = CrossProd(Y1, X1); Z1.normalize();
	//then compute Y axis
	Y0 = CrossProd(Z0, X0); 
	Y1 = CrossProd(Z1, X1); 
	//assign
	facenorm0 = Z0; facenorm1 = Z1;
}


//use the center plane of the dihedral angle as the reference plane
inline void getReferencePlanesForTrianglePair(
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, 
	Vector3d &facenorm0, Vector3d &facenorm1, double3x3& mat, double &xlen)
{
	Vector3d &Z0 = facenorm0;
	Vector3d &Z1 = facenorm1;
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	//compute X-axis
	X = q1; xlen = Magnitude(X); X /= xlen;
	//compute Z-axis
	const Vector3d& Y0 = q2;
	Z0 = CrossProd(X, Y0); Z0.normalize();
	const Vector3d& Y1 = q3;	
	Z1 = CrossProd(Y1, X); Z1.normalize();
	//compute average Z as the mean of Z0 and Z1
	Z = Z0+Z1; Z.normalize();
	//compute Y-axis
	Y = CrossProd(Z, X); 
}

inline void getReferencePlanesForTrianglePair(
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, double3x3& mat, double &xlen)
{
	Vector3d facenorm0, facenorm1;
	getReferencePlanesForTrianglePair(q1, q2, q3, mat, xlen);
}


#define GET_STIFF_VARS(m_stiffness)			\
	const double &A11 = m_stiffness[0].x;	\
	const double &A12 = m_stiffness[0].y;	\
	const double &A22 = m_stiffness[0].z;	\
	const double &B11 = m_stiffness[1].x;	\
	const double &B12 = m_stiffness[1].y;	\
	const double &B22 = m_stiffness[1].z;	

class CTriangle3DPairElement
{
protected:
	double3x3 m_XInv0;		//for a 3D triangle, this is the deriv tensor
	double3x3 m_XInv1;		//for a 3D triangle, this is the deriv tensor
	Vector3d m_stiffness[2];
	//===================================================
	double m_vol0;
	double m_vol1;
	double m_len0;			//...		   pos 1, in the local x axis
	//===================================================
	Vector3d m_F1a;			//force, in the local coord.
	Vector3d m_F1b;	
	Vector3d m_F2;	
	Vector3d m_F3;	
	//===================================================
	Vector3d m_F1tn;		//force, in the gloabal coord. for previous time step
	Vector3d m_F2tn;	
	Vector3d m_F3tn;	

private:
	void _initElement(
		const CMeMaterialProperty &mtl,
		const double3x3& rot0, const double3x3& rot1, 
		const Vector3d& N0, const Vector3d& N1, const double &len0,
		const Vector3d &q1, const Vector3d &q2, const Vector3d &q3); 

public:          
	void init(
		const int rottype, const CMeMaterialProperty &mtl, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const double& thickness);

	void init(
		const double3x3& rot, const CMeMaterialProperty &mtl, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const double& thickness);

	CTriangle3DPairElement(void);

	~CTriangle3DPairElement(void){}

	//The input quad is arranged as follows: normal is pointing out of the plane
	//           P2 / \
	//            /     \
	//          /         \
	//      P0  ------------ P1
	//          \         /
	//           \       /
	//            \    /
	//          P3  \ /
	CTriangle3DPairElement(
		const int rottype, const CMeMaterialProperty &mtl, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3,
		const double thickness)
	{
		init(rottype, mtl, p0, p1, p2, p3, thickness);
	}

	//Compute elastic+bending force for an element in triangle mesh.
	inline void computeForce(
		const double& xlen, const Vector3d &q1, const Vector3d &q2, const Vector3d &q3,
		const double3x3& rot0, const double3x3& rot1,
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d FF[4])					//force in global coord.
	{
		GET_STIFF_VARS(m_stiffness);
		FF[1] = (A11*q1 + A12*q2) + (B11*q1 + B12*q3) - (m_F1a*rot0 + m_F1b*rot1);
		FF[2] = A12*q1 + A22*q2 - m_F2*rot0;
		FF[3] = B12*q1 + B22*q3 - m_F3*rot1;
		//check strain rate dampping
		const double kd = mtl.getEffectiveYoungDamp();
		if (kd > 0.0){
			const Vector3d fd1 = FF[1], fd2 = FF[2], fd3 = FF[3];
			FF[1] += kd*(fd1 - m_F1tn); FF[2] += kd*(fd2 - m_F2tn); FF[3] += kd*(fd3 - m_F3tn);
			m_F1tn = fd1, m_F2tn = fd2, m_F3tn = fd3;
		}
		//compute node 0 force
		FF[0] = -(FF[1]+FF[2]+FF[3]);
	}
	
	inline void computeForce(
		const double& xlen, const Vector3d &q1, const Vector3d &q2, const Vector3d &q3,
		const double3x3& rot,
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d FF[4]) 				//force in global coord.
	{
		GET_STIFF_VARS(m_stiffness);
		FF[1] = (A11*q1 + A12*q2) + (B11*q1 + B12*q3) - (m_F1a + m_F1b)*rot;
		FF[2] = A12*q1 + A22*q2 - m_F2*rot;
		FF[3] = B12*q1 + B22*q3 - m_F3*rot;
		//check strain rate dampping
		const double kd = mtl.getEffectiveYoungDamp();
		if (kd > 0.0){
			const Vector3d fd1 = FF[1], fd2 = FF[2], fd3 = FF[3];
			FF[1] += kd*(fd1 - m_F1tn); FF[2] += kd*(fd2 - m_F2tn); FF[3] += kd*(fd3 - m_F3tn);
			m_F1tn = fd1, m_F2tn = fd2, m_F3tn = fd3;
		}
		//compute node 0 force
		FF[0] = -(FF[1]+FF[2]+FF[3]);
	}

	double getStiffnessRatio(void) const;

	inline double* getStiffness(void)
	{
		return &m_stiffness[0].x;
	}

	friend class CThinshellElementFEM;
};

#endif