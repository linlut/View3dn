//FILE: element_tetfem2.h
//Using two 3D triangle element for each tet element

#ifndef _INC_FEMTET3_ZN_20091028H
#define _INC_FEMTET3_ZN_20091028H

#include "element_base.h"
#include "element_trianglepair.h"

class CSparseMatrix33;

class CTetFem3Element: public CElementBase
{
protected:
	int m_nodeID[4];				//The 4 nodes wich form the quad
	CTriangle3DPairElement tri0;
	CTriangle3DPairElement tri1;
	//data members for processing flipping
	bool m_bFlipProcessed;
	unsigned char m_flipEdgeID;
	bool m_bPadding[2];
	Vector3d m_normal[4];

private:
	void _initElement(const CMeMaterialProperty& mtl, const double &thickness,
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3);

public:          
	void init(const CMeMaterialProperty& mtl, const int *nodeid, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3);

	CTetFem3Element(void): CElementBase(), tri0(), tri1()
	{}

	~CTetFem3Element(void){}

	CTetFem3Element(const CMeMaterialProperty& mtl, const int* pNodeIDBuffer, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3):
		CElementBase(), tri0(), tri1()
	{
		init(mtl, pNodeIDBuffer, p0, p1, p2, p3);
	}

	const int* getElementNodeBuffer(void) const
	{
		return m_nodeID;
	}

	inline double getStiffnessRatio(void)
	{
		return tri0.getStiffnessRatio()+tri1.getStiffnessRatio();
	}

	void computeForce(
		const int timeid, const int isstatic,
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const CMeMaterialProperty &mtl, 
		Vector3d F[4], double3x3* jac[12]);

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[]);

	static bool hasConstantStiffness(void){ return true; }

	static bool isStiffnessMatrix3X3(void){ return false; }

};

#endif