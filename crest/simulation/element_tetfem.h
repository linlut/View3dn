//FILE: element_tetfem.h
//Corotational FEM implementation

#ifndef _INC_FEMTET_ZN_200902H
#define _INC_FEMTET_ZN_200902H

#include <math.h>
#include <vector3d.h>
#include <matrix3x3.h>
#include "mechanical_property.h"
#include "element_base.h"


//check the element orientation, where the orientation of a tet element is specified as:
// sign = sign((p1-p0)X(p2-p0) * (p3-p0))
// true: sign>0, else false
inline bool positiveOrientation(const Vector3d& p0, const Vector3d& p1, const Vector3d& p2, const Vector3d& p3)
{
	const double r = DotProd(p3-p0, CrossProd(p1-p0, p2-p0));
	bool flag = (r>0)? true: false;
	return flag;
}

class CTetFemElement: public CElementBase
{
public:          
	void init(const CMeMaterialProperty& mtl, const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
	{
		m_nodeID[0] = nodeid[0];
		m_nodeID[1] = nodeid[1];
		m_nodeID[2] = nodeid[2];
		m_nodeID[3] = nodeid[3];
		m_vert[0] = Vector3d(0,0,0);
		m_vert[1] = p1 - p0;
		m_vert[2] = p2 - p0;
		m_vert[3] = p3 - p0;
		_initElement(mtl, p0, p1, p2, p3);
	}

	CTetFemElement(void): CElementBase(){}

	~CTetFemElement(void){}

	CTetFemElement(const CMeMaterialProperty& mtl, const int* pNodeIDBuffer, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3):
		CElementBase()
	{
		init(mtl, pNodeIDBuffer, p0, p1, p2, p3);
	}

	const int* getElementNodeBuffer(void) const
	{
		return m_nodeID;
	}

	void computeForce(
		const unsigned int timeid, 
		const int isstatic, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const CMeMaterialProperty &mtl, 
		Vector3d F[4], double3x3* jac[4]);

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[9]);

	static bool hasConstantStiffness(void)
	{ 
		return false; 
	}

	static bool isStiffnessMatrix3X3(void)
	{ 
		return true; 
	}

private:
	int m_nodeID[4];				//The 4 nodes wich form the quad
	double m_vol;					//volume of the tet
	Vector3d m_vert[4];				//position of the undeformed vertex state
	double3x3 m_matXT;				//X^(-1)
	double3x3 m_matRot;				//buffer a rotation matrix to improve the speed
	double3x3 m_matStiff[9];		//stiffness matrix for the 4 vertices

private:
	void _initElement(const CMeMaterialProperty& mtl,const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3);
	void _computeForceStatic(const double3x3&DG, const CMeMaterialProperty &mtl, Vector3d F[3]);
	void _computeRotation(const unsigned int timeid, const int isstatic, 
			const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, 
			double3x3 &R,	//local to world rotation matrix
			double3x3 &S);	//stretch component
};

#endif