//FILE: thinsehll_element.h

#ifndef _INC_THINSHELL_ELM_H
#define _INC_THINSHELL_ELM_H

#include "shear_element.h"
#include "gspring_element.h"

void GenReferencePlanesForTrianglePair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	double3x3& mat, double3x3& mat2);

void GenReferencePlanesForQuadPair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	double3x3& mat, double3x3& mat2);


template<class T> 
class CThinshellElementT
{
private:
	int m_nNodeID[6];	//The 4 nodes wich form the quad, or 8 nodes which share a common edge
	T m_shearelm[4];	//The 4 shear elements constructed from the quad

private:
	//thickness: the thickness of the shell element
	//shear element composed of two triangles
	void _initShearElements(const int isStatic, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double& thickness)
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
		double3x3 mat, mat2;
		if (isStatic)
			mat.IdentityMatrix(), mat2.IdentityMatrix();
		else
			GenReferencePlanesForTrianglePair(p0, p1, p2, p3, mat, mat2);
		m_shearelm[0].init(p0, p2, mat2, trussvol1);
		m_shearelm[1].init(p1, p2, mat2, trussvol1);
		m_shearelm[2].init(p0, p3, mat, trussvol2);
		m_shearelm[3].init(p1, p3, mat, trussvol2);
	}

	//shear element is composed of two quads
	void _initShearElements(const int isStatic, 
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
		double3x3 mat, mat2;
		if (isStatic)
			mat.IdentityMatrix(), mat2.IdentityMatrix();
		else
			GenReferencePlanesForQuadPair(p0, p1, p2, p3, p4, p5, mat, mat2);
		m_shearelm[0].init(p0, p3, mat2, trussvol1);
		m_shearelm[1].init(p1, p2, mat2, trussvol1);
		m_shearelm[2].init(p0, p4, mat, trussvol2);
		m_shearelm[3].init(p1, p5, mat, trussvol2);
	}

public:          
	void init(const int isstatic, const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double& thickness)
	{
		m_nNodeID[0] = nodeid[0];
		m_nNodeID[1] = nodeid[1];
		m_nNodeID[2] = nodeid[2];
		m_nNodeID[3] = nodeid[3];
		m_nNodeID[4] = m_nNodeID[5] = 0;
		_initShearElements(isstatic, p0, p1, p2, p3, thickness);
	}

	void init(const int isstatic, const int *nodeid, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, const double& thickness)
	{
		m_nNodeID[0] = nodeid[0];
		m_nNodeID[1] = nodeid[1];
		m_nNodeID[2] = nodeid[2];
		m_nNodeID[3] = nodeid[3];
		m_nNodeID[4] = nodeid[4];
		m_nNodeID[5] = nodeid[5];
		_initShearElements(isstatic, p0, p1, p2, p3, p4, p5, thickness);
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
	CThinshellElementT(const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double thickness)
	{
		init(isstatic, pNodeIDBuffer, p0, p1, p2, p3, thickness);
	}

	CThinshellElementT(const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, const double thickness)
	{
		init(isstatic, pNodeIDBuffer, p0, p1, p2, p3, p4, p5, thickness);
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
			double3x3 rotmat, rotmat2;
			GenReferencePlanesForTrianglePair(p0, p1, p2, p3, rotmat, rotmat2);
			//compute local forces for each shear element
			m_shearelm[0].computeNodalForce(p0, p2, rotmat2, mtl, ff[0], ppJacobian[0]);
			m_shearelm[1].computeNodalForce(p1, p2, rotmat2, mtl, ff[1], ppJacobian[1]);
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
			double3x3 rotmat, rotmat2;
			GenReferencePlanesForQuadPair(p0, p1, p2, p3, p4, p5, rotmat, rotmat2);
			//compute local forces for each shear element
			m_shearelm[0].computeNodalForce(p0, p3, rotmat2, mtl, ff[0], ppJacobian[0]);
			m_shearelm[1].computeNodalForce(p1, p2, rotmat2, mtl, ff[1], ppJacobian[1]);
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

	void updateState(void){}

};


typedef CThinshellElementT<CShearElement> CThinshellElement;
typedef CThinshellElementT<CGspringElement> CThinshellSpringElement;


#endif