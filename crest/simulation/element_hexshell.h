//FILE: element_hexshell.h
//Using two 3D triangle element for each hexshell element

#ifndef _INC_HEXSHELL_ZN_20091103H
#define _INC_HEXSHELL_ZN_20091103H

#include "element_base.h"
#include "thinshell3_element.h"

class CSparseMatrix33;

class CHexshellElement: public CElementBase
{
protected:
	int m_nodeID[8];				//The 8 nodes wich form the hexahedron
	CThinshellElementFEM qpair0;
	CThinshellElementFEM qpair1;
	CThinshellElementFEM qpair2;

private:
	void _initElement(const CMeMaterialProperty& mtl, const double &thickness,
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3,
		const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7);

public:          
	void init(const CMeMaterialProperty& mtl, const int *nodeid, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3,
		const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7);

	CHexshellElement(void): 
		CElementBase(), qpair0(), qpair1(), qpair2()
	{}

	~CHexshellElement(void){}

	CHexshellElement(const CMeMaterialProperty& mtl, const int* pNodeIDBuffer, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7):
		CElementBase(), qpair0(), qpair1(), qpair2()
	{
		init(mtl, pNodeIDBuffer, p0, p1, p2, p3, p4, p5, p6, p7);
	}

	const int* getElementNodeBuffer(void) const
	{
		return m_nodeID;
	}

	void computeForce(
		const int timeid, const int isstatic,
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7,
		const CMeMaterialProperty &mtl, 
		Vector3d F[8], double3x3* jac[39]);

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[]);

	static bool hasConstantStiffness(void){ return true; }

};

#endif