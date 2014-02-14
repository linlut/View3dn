//FILE: thinsehll3_element.h

#ifndef _INC_FASTFEM_THINSHELL_ELM_H
#define _INC_FASTFEM_THINSHELL_ELM_H


#include "element_trianglepair.h"
#include "element_base.h"


class CThinshellElementFEM: public CElementBase
{
private:
	int m_nNodeID[6];			//The 4 nodes wich form the quad, or 8 nodes which share a common edge
	CTriangle3DPairElement tri0;
	CTriangle3DPairElement tri1;

private:
	//thickness: the thickness of the shell element
	//shear element composed of two triangles
	void _initElements(
		const CMeMaterialProperty &mtl, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const double& thickness);

	//shear element is composed of two quads
	void _initElements(
		const CMeMaterialProperty &mtl, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
		const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
		const double& thickness);

	//chnage stiffness in some cases
	//void _checkStiffness(const double& S12, double& S11, double& S22);

public:          
	void init(const CMeMaterialProperty &mtl, const int isstatic, const int *nodeid, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const double& thickness)
	{
		m_nNodeID[4] = m_nNodeID[5] = -1;
		const double d0 = fabs(DotProd(p1-p0, p2-p0))+fabs(DotProd(p1-p0, p3-p0));
		const double d1 = fabs(DotProd(p0-p1, p2-p1))+fabs(DotProd(p0-p1, p3-p1));
		if (d0>=d1){
			m_nNodeID[0] = nodeid[0]; m_nNodeID[1] = nodeid[1];
			m_nNodeID[2] = nodeid[2]; m_nNodeID[3] = nodeid[3];
			_initElements(mtl, p0, p1, p2, p3, thickness);
		}
		else{
			m_nNodeID[0] = nodeid[1]; m_nNodeID[1] = nodeid[0];
			m_nNodeID[2] = nodeid[3]; m_nNodeID[3] = nodeid[2];
			_initElements(mtl, p1, p0, p3, p2, thickness);
		}
	}

	void init(const CMeMaterialProperty &mtl, const int isstatic, const int *nodeid, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
		const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
		const double& thickness)
	{
		m_nNodeID[0] = nodeid[0];
		m_nNodeID[1] = nodeid[1];
		m_nNodeID[2] = nodeid[2];
		m_nNodeID[3] = nodeid[3];
		m_nNodeID[4] = nodeid[4];
		m_nNodeID[5] = nodeid[5];
		_initElements(mtl, p0, p1, p2, p3, p4, p5, thickness);
	}

	CThinshellElementFEM(void): CElementBase(), tri0(), tri1(){}

	~CThinshellElementFEM(void){}

	//The input quad is arranged as follows:
	//           P2 / \
	//            /     \
	//          /         \
	//      P0  ------------ P1
	//          \         /
	//           \       /
	//            \    /
	//          P3  \ /
	CThinshellElementFEM(const CMeMaterialProperty &mtl, const int isstatic, const int* pNodeIDBuffer, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const double thickness):
		CElementBase(), tri0(), tri1()
	{
		init(mtl, isstatic, pNodeIDBuffer, p0, p1, p2, p3, thickness);
	}

	//The input quad is arranged as follows:
	//           P3 ---P2
	//           /       \
	//          /         \
	//      P0  ------------ P1
	//          \         /
	//           \       /
	//           \      /
	//          P4 -----P5
	CThinshellElementFEM(
		const CMeMaterialProperty &mtl, const int isstatic, const int* pNodeIDBuffer, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
		const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
		const double thickness):
		tri0(), tri1()
	{
		init(mtl, isstatic, pNodeIDBuffer, p0, p1, p2, p3, p4, p5, thickness);
	}

	inline const int* getElementNodeBuffer(void) const
	{
		return m_nNodeID;
	}

	//compute elastic+bending force for a shear element in triangle mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	inline void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const int isStatic,				//static simulation or dynamic simu
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d FF[4],					//force in global coord.
		double3x3 *ppJacobian[7])		//jocobian matrices, if needed (not NULL)
	{
		//generate the two world/local transform matrices
		double xlen;
		double3x3 rot;
		Vector3d N0, N1;
		const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
		getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot, xlen);
		tri0.computeForce(xlen, q1, q2, q3, rot, mtl, FF);
		//copy stiffness since it is constant
		if (ppJacobian[0]){
			const double *stiff = tri0.getStiffness();
			const double kd = mtl.getEffectiveYoungDamp();
			const double kd1 = 1.0 + kd;
			for (int i=0; i<6; i++) ppJacobian[i]->x[0]=stiff[i]*kd1;
			//add spring stiffness
			ppJacobian[6]->setZeroMatrix();
			double *x = ppJacobian[6]->x; 
			x[0]= x[4] = x[8] = 0;
		}
	}

	//compute elastic+bending force for a shear element in quad mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
		const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
		const int isStatic,				//static simulation or dynamic simu
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d F[6],					//force in global coord.
		double3x3 *ppJacobian[13]);		//jocobian matrices, if needed (not NULL)


	inline void updateState(void)
	{
	}

	void plastifyItself(const double& th)
	{
		/*
		Tensor3x3* F; 
		F = m_shearelm[0].getDeformationGradient();
		if (F) F->IdentityMatrix();
		F = m_shearelm[1].getDeformationGradient();
		if (F) F->IdentityMatrix();
		F = m_shearelm[2].getDeformationGradient();
		if (F) F->IdentityMatrix();
		F = m_shearelm[3].getDeformationGradient();
		if (F) F->IdentityMatrix();
		*/
	}

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[]);

	static bool isSpringElement(void){ return false; }

	static inline bool hasConstantStiffness(void)
	{ 
		return true; 
	}

	static bool isStiffnessMatrix3X3(void)
	{
		return false;
	}

};


#endif