//FILE: thinsehll2_element.h
//A new fast thin shell element def. using the per vertex linear invariant coor. 

#ifndef _INC_FAST_THINSHELL2_ELM_H
#define _INC_FAST_THINSHELL2_ELM_H

#include "element_shear.h"

#define MAX_NODE_VALENCE 10

class CSimuThinshell2;

class CThinshell2Element
{
protected:
	int m_nCenterID;							//center node id
	int m_nRod;									//# of 1-ring neighbour nodes, also # of rod 
	int m_nNodeID[MAX_NODE_VALENCE];			//first node--center node, remaining are the 1-ring
	int m_n1RingPoly;							//# of 1-ring neighbour polygons
	int m_n1RingPolyID[MAX_NODE_VALENCE];		//1-ring polygon IDs
	CShearElement m_shearelm[MAX_NODE_VALENCE];	//shear elements constructed from the quad

private:
	//thickness: the thickness of the shell element
	//shear element composed of two triangles
	void _initShearElements(const int isStatic, 
		const CMeMaterialProperty& mtl,	
		const Vector3d p[], const Vector3d wpnorm[], const double trussvolume[]);

	void _computeLocalTransformMatrix2Tagent(const Vector3d p[], const int stride, const Vector3d norm[], double3x3& mat);

public:          
	void init(const int isstatic, 
		const CMeMaterialProperty& mtl,	
		const int nodeid,						//the center vertex id
		const Vector3d *p,						//vertex position buffer
		const Vector3d *wpnorm,					//weighted surface polygon normal array, NOTE: the whole mesh!!!!
		const int *surfpoly,					//buffer of the whole surface polygons
		const int nv_per_poly,					//number of vertices for each surface polygon
		const int *polyfanid,					//the polyfan around the center vertex, input IDs only
		const int nValence,						//valence of the center vertex, also length of poylfanid
		const double& thickness);

	CThinshell2Element(void)
	{
		m_n1RingPoly = 0;
		m_nRod = 0;
	}

	~CThinshell2Element(void){}

	CThinshell2Element(const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const double thickness)
	{
		//init(isstatic, pNodeIDBuffer, p0, p1, p2, p3, thickness);
	}

	CThinshell2Element(const int isstatic, const int* pNodeIDBuffer, const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, const double thickness)
	{
		//init(isstatic, pNodeIDBuffer, p0, p1, p2, p3, p4, p5, thickness);
	}

	const int* getElementNodeBuffer(void) const
	{
		return m_nNodeID;
	}

	const int getRodNumber(void) const
	{
		return m_nRod;					
	}

	const int get1RingPolygonNumber(void) const
	{
		return m_n1RingPoly;			
	}

	//compute elastic+bending force for a shear element in triangle mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	void computeForce(
		const int isStatic,					//static simulation or dynamic simu	
		const Vector3d pvert[],				//nodes' current positions
		const int stride,					//consider nodes are packed in an array which are not continious
		const Vector3d pnormal[],			//an input normal array for the whole mesh, not only the element
		const CMeMaterialProperty &mtl,		//material
		Vector3d F[],						//force array
		double3x3 *ppJacobian[]);			//jacobian array

	friend class CSimuThinshell2;

	static bool hasConstantStiffness(void){ return false; }

	static bool isStiffnessMatrix3X3(void){ return false; }

};

#endif