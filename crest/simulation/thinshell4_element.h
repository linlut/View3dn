//FILE: thinsehll4_element.h
//Note: for the Bridson's alg.

#ifndef _INC_BRIDSONTHINSHELL_ELM_H
#define _INC_BRIDSONTHINSHELL_ELM_H

#include "element_base.h"
#include "mechanical_property.h"

class CSparseMatrix33;
class CHindgeElement
{
protected:
	double m_sintheta0;

public:
	void init(const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double& thickness);
	CHindgeElement(void);
	~CHindgeElement(){}

	void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const CMeMaterialProperty &mtl,		//material parameters
		Vector3d F[4],						//force in global coord.
		double3x3 *ppJacobian[16]);			//jocobian matrices, if needed (not NULL)

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[]);
};



class CThinshellBridsonElement: public CElementBase
{
private:
	int m_nNodeID[6];	//The 4 nodes wich form the quad
	CHindgeElement m_hindge[2];

public:          
	void init(const CMeMaterialProperty &mtl, const int isstatic, const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double& thickness)
	{
		int &b1 =  m_nNodeID[2];
		int &b2 =  m_nNodeID[3];
		int &b3 =  m_nNodeID[0];
		int &b4 =  m_nNodeID[1];
		b1 = nodeid[0];
		b2 = nodeid[1];
		b3 = nodeid[2];
		b4 = nodeid[3];
		m_nNodeID[4] = m_nNodeID[5] = 0;
		m_hindge[0].init(p2, p3, p0, p1, thickness);
	}

	void init(const CMeMaterialProperty &mtl, const int isstatic, const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, const double& thickness)
	{ assert(0); }

	CThinshellBridsonElement(void){}

	~CThinshellBridsonElement(void){}

	//The input quad is arranged as follows:
	//           P2 / \
	//            /     \
	//          /         \
	//      P0  ------------ P1
	//          \         /
	//           \       /
	//            \    /
	//          P3  \ /
	CThinshellBridsonElement(const CMeMaterialProperty &mtl, const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double thickness)
	{
		init(mtl, isstatic, pNodeIDBuffer, p0, p1, p2, p3, thickness);
	}

	CThinshellBridsonElement(const CMeMaterialProperty &mtl, const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, const double thickness)
	{
		init(mtl, isstatic, pNodeIDBuffer, p0, p1, p2, p3, p4, p5, thickness);
	}

	inline const int* getElementNodeBuffer(void) const
	{
		return m_nNodeID;
	}

	void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const int isStatic,				//static simulation or dynamic simu
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d F[4],					//force in global coord.
		double3x3 *ppJacobian[16])		//jocobian matrices, if needed (not NULL)
	{
		m_hindge[0].computeForce(p0, p1, p2, p3, mtl, F, ppJacobian);
	}

	//compute elastic+bending force for a shear element in quad mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	inline void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
		const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
		const int isStatic,				//static simulation or dynamic simu
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d F[6],					//force in global coord.
		double3x3 *ppJacobian[16])		//jocobian matrices, if needed (not NULL)
	{
		assert(0);
	}

	inline void updateState(void){}

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[]);

	void plastifyItself(const double& th)
	{
		assert(0);
	}

	static bool isSpringElement(void)
	{ 
		return false; 
	}

	static bool hasConstantStiffness(void)
	{ 
		return false;
	}

	static bool isStiffnessMatrix3X3(void)
	{ 
		return true; 
	}

};


#endif