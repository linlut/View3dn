//FILE: thinsehll_element.h
//NOTE: for spring and angular spring based thin shell elements

#ifndef _INC_THINSHELL_ELM_H
#define _INC_THINSHELL_ELM_H

#include "element_shear.h"
#include "element_simp_shear.h"
#include "gspring_element.h"
#include "element_gyrod.h"


//construct the two local coord sys. of the shell element using current nodal positions
//the aixs of the coord sys are stored in two 3x3 matrices
//static inline 
inline void GenReferencePlanesForTrianglePair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	double3x3& mat)
{
	Vector3d Y0, Y1, Z0, Z1;
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	X = p1 - p0; X.normalize();
	Y0 = p2 - p0;
	Z0 = CrossProd(X, Y0); 
	Y1 = p3 - p0;	
	Z1 = CrossProd(Y1, X); 
	Z0.normalize();
	Z1.normalize();
	Z = Z0+Z1; Z.normalize();
	Y = CrossProd(Z, X); 
}


//use the center plane of the dihedral angle as the reference plane
inline void GenReferencePlanesForQuadPair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	double3x3& mat)
{
	Vector3d Y0, Y1, Z0, Z1;
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	const Vector3d p01 = (p0+p1);
	const Vector3d p23 = (p2+p3+p01)*0.25;
	const Vector3d p45 = (p4+p5+p01)*0.25;
	X = p1 - p0; X.normalize();
	Y0 = p23 - p0;
	Z0 = CrossProd(X, Y0); 
	Y1 = p45 - p0;	
	Z1 = CrossProd(Y1, X); 
	Z0.normalize();
	Z1.normalize();
	Z = Z0+Z1; Z.normalize();
	Y = CrossProd(Z, X); 
}


template<class T> 
class CThinshellElementT
{
private:
	int m_nNodeID[6];	//The 4 nodes wich form the quad, or 8 nodes which share a common edge
	T m_shearelm[4];	//The 4 shear elements constructed from the quad

private:
	//thickness: the thickness of the shell element
	//shear element composed of two triangles
	void _initShearElements(const int isStatic, 
		const CMeMaterialProperty &mtl,
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const double& thickness)
	{
		//compute volume for each triangle, each truss edge
		const double onethird = 0.33333333333333333333333333333;
		const double a1 = triangle_area(p0, p1, p2);
		const double a2 = triangle_area(p0, p1, p3);
		const double vol1 = a1*thickness;
		const double vol2 = a2*thickness;
		const double trussvol1 = vol1*onethird;
		const double trussvol2 = vol2*onethird;

		//construct the world/local transform matrix for the two triangles
		double3x3 mat;
		if (isStatic)
			mat.setIdentityMatrix();
		else
			GenReferencePlanesForTrianglePair(p0, p1, p2, p3, mat);
		m_shearelm[0].init(mtl, p0, p2, mat, trussvol1);
		m_shearelm[1].init(mtl, p1, p2, mat, trussvol1);
		m_shearelm[2].init(mtl, p0, p3, mat, trussvol2);
		m_shearelm[3].init(mtl, p1, p3, mat, trussvol2);
	}

	//shear element is composed of two quads
	void _initShearElements(const int isStatic,
		const CMeMaterialProperty &mtl,
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
		const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
		const double& thickness)
	{
		//compute volume for each triangle, each truss edge
		const double onethird = 0.25;
		const double a1 = triangle_area(p0, p1, p2)+triangle_area(p0, p2, p3);
		const double a2 = triangle_area(p0, p4, p1)+triangle_area(p1, p4, p5);
		const double vol1 = a1*thickness;
		const double vol2 = a2*thickness;
		const double trussvol1 = vol1*onethird;
		const double trussvol2 = vol2*onethird;

		//construct the world/local transform matrix for the two triangles
		double3x3 mat;
		if (isStatic)
			mat.setIdentityMatrix();
		else
			GenReferencePlanesForQuadPair(p0, p1, p2, p3, p4, p5, mat);
		m_shearelm[0].init(mtl, p0, p3, mat, trussvol1);
		m_shearelm[1].init(mtl, p1, p2, mat, trussvol1);
		m_shearelm[2].init(mtl, p0, p4, mat, trussvol2);
		m_shearelm[3].init(mtl, p1, p5, mat, trussvol2);
	}

public:          
	void init(const CMeMaterialProperty &mtl, const int isstatic, const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double& thickness)
	{
		m_nNodeID[0] = nodeid[0];
		m_nNodeID[1] = nodeid[1];
		m_nNodeID[2] = nodeid[2];
		m_nNodeID[3] = nodeid[3];
		m_nNodeID[4] = m_nNodeID[5] = 0;
		_initShearElements(isstatic, mtl, p0, p1, p2, p3, thickness);
	}

	void init(const CMeMaterialProperty &mtl, const int isstatic, const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, const double& thickness)
	{
		m_nNodeID[0] = nodeid[0];
		m_nNodeID[1] = nodeid[1];
		m_nNodeID[2] = nodeid[2];
		m_nNodeID[3] = nodeid[3];
		m_nNodeID[4] = nodeid[4];
		m_nNodeID[5] = nodeid[5];
		_initShearElements(isstatic, mtl, p0, p1, p2, p3, p4, p5, thickness);
	}

	CThinshellElementT(void){}

	~CThinshellElementT(void){}

	//The input quad is arranged as follows:
	//           P2 / \
	//            /     \
	//          /         \
	//      P0  ------------ P1
	//          \         /
	//           \       /
	//            \    /
	//          P3  \ /
	CThinshellElementT(const CMeMaterialProperty &mtl, const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double thickness)
	{
		init(mtl, isstatic, pNodeIDBuffer, p0, p1, p2, p3, thickness);
	}

	CThinshellElementT(const CMeMaterialProperty &mtl, const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, const double thickness)
	{
		init(mtl, isstatic, pNodeIDBuffer, p0, p1, p2, p3, p4, p5, thickness);
	}

	inline const int* getElementNodeBuffer(void) const
	{
		return m_nNodeID;
	}
/*
	void computeForceBridson(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const int isStatic,				//static simulation or dynamic simu
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d F[4],					//force in global coord.
		double3x3 *ppJacobian[4])		//jocobian matrices, if needed (not NULL)
	{
		extern void computeBendingForceBridson(
			const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, 
			const int isStatic,				//static simulation or dynamic simu
			const CMeMaterialProperty &mtl, //material parameters
			Vector3d F[4],					//force in global coord.
			double3x3 *ppJacobian[4]);		//jocobian matrices, if needed (not NULL)
		Vector3d f[4];
		computeBendingForceBridson(p2, p3, p0, p1, isStatic, mtl, f, ppJacobian);
		F[0]+=f[2];
		F[1]+=f[3];
		F[2]+=f[0];
		F[3]+=f[1];
	}
*/
	//compute elastic+bending force for a shear element in triangle mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	inline void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const int isStatic,				//static simulation or dynamic simu
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d F[4],					//force in global coord.
		double3x3 *ppJacobian[4])		//jocobian matrices, if needed (not NULL)
	{
		Vector3d ff[4];					//global elastic force for each element

		if (isStatic){
			m_shearelm[0].computeNodalForce(p0, p2, mtl, ff[0], ppJacobian[0]);
			m_shearelm[1].computeNodalForce(p1, p2, mtl, ff[1], ppJacobian[1]);
			m_shearelm[2].computeNodalForce(p0, p3, mtl, ff[2], ppJacobian[2]);
			m_shearelm[3].computeNodalForce(p1, p3, mtl, ff[3], ppJacobian[3]);
		}
		else{
			//gen. the world/local and local/world transformation matrices
			double3x3 rotmat;
			GenReferencePlanesForTrianglePair(p0, p1, p2, p3, rotmat);
			//compute local forces for each shear element
			m_shearelm[0].computeNodalForce(p0, p2, rotmat, mtl, ff[0], ppJacobian[0]);
			m_shearelm[1].computeNodalForce(p1, p2, rotmat, mtl, ff[1], ppJacobian[1]);
			m_shearelm[2].computeNodalForce(p0, p3, rotmat, mtl, ff[2], ppJacobian[2]);
			m_shearelm[3].computeNodalForce(p1, p3, rotmat, mtl, ff[3], ppJacobian[3]);
		}

		//accumulate forces on the nodes
		F[0] = ff[0]+ff[2];
		F[1] = ff[1]+ff[3];
		F[2] = -(ff[0]+ff[1]);
		F[3] = -(ff[2]+ff[3]);
	}

	//compute elastic+bending force for a shear element in quad mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	inline void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
		const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
		const int isStatic,				//static simulation or dynamic simu
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d F[6],					//force in global coord.
		double3x3 *ppJacobian[4])		//jocobian matrices, if needed (not NULL)
	{
		Vector3d ff[4];					//global elastic force for each element

		if (isStatic){
			m_shearelm[0].computeNodalForce(p0, p3, mtl, ff[0], ppJacobian[0]);
			m_shearelm[1].computeNodalForce(p1, p2, mtl, ff[1], ppJacobian[1]);
			m_shearelm[2].computeNodalForce(p0, p4, mtl, ff[2], ppJacobian[2]);
			m_shearelm[3].computeNodalForce(p1, p5, mtl, ff[3], ppJacobian[3]);
		}
		else{
			//gen. the world/local and local/world transformation matrices
			double3x3 rotmat;
			GenReferencePlanesForQuadPair(p0, p1, p2, p3, p4, p5, rotmat);
			//compute local forces for each shear element
			m_shearelm[0].computeNodalForce(p0, p3, rotmat, mtl, ff[0], ppJacobian[0]);
			m_shearelm[1].computeNodalForce(p1, p2, rotmat, mtl, ff[1], ppJacobian[1]);
			m_shearelm[2].computeNodalForce(p0, p4, rotmat, mtl, ff[2], ppJacobian[2]);
			m_shearelm[3].computeNodalForce(p1, p5, rotmat, mtl, ff[3], ppJacobian[3]);
		}

		//accumulate forces on the nodes
		F[0] = ff[0]+ff[2];
		F[1] = ff[1]+ff[3];
		F[2] = -ff[1];
		F[3] = -ff[0];
		F[4] = -ff[2];
		F[5] = -ff[3];
	}

	inline void updateState(void)
	{
	}

	void plastifyItself(const double& th)
	{
		Tensor3x3* F; 
		F = m_shearelm[0].getDeformationGradient();
		if (F) F->setIdentityMatrix();
		F = m_shearelm[1].getDeformationGradient();
		if (F) F->setIdentityMatrix();
		F = m_shearelm[2].getDeformationGradient();
		if (F) F->setIdentityMatrix();
		F = m_shearelm[3].getDeformationGradient();
		if (F) F->setIdentityMatrix();
	}

	static bool isSpringElement(void){ return true; }

	static bool hasConstantStiffness(void)
	{ 
		const bool r = T::hasConstantStiffness();
		return r;
	}

	static bool isStiffnessMatrix3X3(void)
	{ 
		const bool r = T::isStiffnessMatrix3X3();
		return r; 
	}

};


typedef CThinshellElementT<CShearElement> CThinshellShearElement;
typedef CThinshellElementT<CSimpShearElement> CThinshellSimpShearElement;
typedef CThinshellElementT<CGspringElement> CThinshellSpringElement;
typedef CThinshellElementT<CGyrodElement> CThinshellGyrodElement;

typedef CThinshellShearElement CThinshellElement;

#endif