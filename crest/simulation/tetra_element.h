//FILE: element_tetfem.h

#ifndef _INC_TETRAHEDRON_SHEAR2_ELM_H
#define _INC_TETRAHEDRON_SHEAR2_ELM_H

/*
#include "shear_element.h"

class CTetra2Element
{
private:
	int m_nodeID[4];				//The 4 nodes wich form the quad
	CShearElement m_shearelm[4];

private:
	//shear element composed of two triangles
	void _initShearElements(const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3);

public:          
	void init(const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
	{
		m_nodeID[0] = nodeid[0];
		m_nodeID[1] = nodeid[1];
		m_nodeID[2] = nodeid[2];
		m_nodeID[3] = nodeid[3];
		_initShearElements(p0, p1, p2, p3);
	}

	CTetra2Element(void){}

	~CTetra2Element(void){}

	CTetra2Element(const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
	{
		init(pNodeIDBuffer, p0, p1, p2, p3);
	}

	inline const int* getElementNodeBuffer(void) const
	{
		return m_nodeID;
	}

	void computeForce(const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const CMeMaterialProperty &mtl, Vector3d F[4]);

};

*/
#endif