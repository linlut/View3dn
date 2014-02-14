//FILE: element_tetfem.h
//Corotational FEM implementation

#ifndef _INC_FEMHEX_ZN_201101H
#define _INC_FEMHEX_ZN_201101H

#include <math.h>
#include <vector3d.h>
#include <matrix3x3.h>
#include "mechanical_property.h"
#include "element_base.h"


class CCubeFemElement: public CElementBase
{
public:
	struct UserData
	{
		double3x3 mStiffnessMatrix[8][8];
	};

	void init(const CMeMaterialProperty& mtl, const int nodeid[8], 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7, 
		void *usrdata);

	CCubeFemElement(void): CElementBase(){}

	~CCubeFemElement(void){}

	CCubeFemElement(const CMeMaterialProperty& mtl, const int* pNodeIDBuffer, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7, 
		void *userdata):	//stiffness template or other acceleration structures
		CElementBase()
	{
		init(mtl, pNodeIDBuffer, p0, p1, p2, p3, p4, p5, p6, p7, userdata);
	}

	const int* getElementNodeBuffer(void) const
	{
		return m_nodeID;
	}

	void computeForce(
		const unsigned int timeid, 
		const int isstatic, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7, 
		const CMeMaterialProperty &mtl, 
		Vector3d F[8], 
		double3x3* jac[64]);

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[64]);

	static bool hasConstantStiffness(void)
	{ 
		return false; 
	}

	static bool isStiffnessMatrix3X3(void)
	{ 
		return true; 
	}

protected:
	int m_nodeID[8];				//The 4 nodes wich form the quad
	UserData *m_pUserData;			//pointer to stiffness template
	Vector3d m_vert[8];				//position of the undeformed vertex state
	double3x3 m_matRot;

private:
	void _computeForceStatic(
			const Vector3d q[8], const CMeMaterialProperty &mtl, 
			Vector3d F[3]);
	void _computeRotation(
			const unsigned int timeid, const int isstatic, 
			const Vector3d q[8], double3x3 &R);	//local to world rotation matrix
};


//compute the stiffness matrix of a cube element
void buildCubeElementStiffness(
	const double &h, 		//cube edge length
	const double &E, 		//Young's modulus
	const double &miu,		//poisson ratio
	double3x3 K[8][8]);		//stiffness matrix of the element

#endif