//FILE: element_tetfem2.h
//Using the Green strain for tet element

#ifndef _INC_FEMTET2_ZN_20090828H
#define _INC_FEMTET2_ZN_20090828H

#include <math.h>
#include <vector3d.h>
#include <matrix3x3.h>
#include "mechanical_property.h"
#include "element_base.h"


class CTetFem2Element: public CElementBase
{
private:
	int m_nodeID[4];				//The 4 nodes wich form the quad
	double m_vol;					//volume of the tet
	double3x3 m_matInvX;			//X^(-1)

private:
	void _initElement(const CMeMaterialProperty& mtl,const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3);

public:          
	void init(const CMeMaterialProperty& mtl, const int *nodeid, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
	{
		m_nodeID[0] = nodeid[0];
		m_nodeID[1] = nodeid[1];
		m_nodeID[2] = nodeid[2];
		m_nodeID[3] = nodeid[3];
		_initElement(mtl, p0, p1, p2, p3);
	}

	CTetFem2Element(void): CElementBase(){}

	~CTetFem2Element(void){}

	CTetFem2Element(const CMeMaterialProperty& mtl, const int* pNodeIDBuffer, 
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
		const int timeid, const int isStatic,
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const CMeMaterialProperty &mtl, 
		Vector3d F[4], double3x3* jac[4]);

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[]){}


};

#endif