//FILE: vertexring_element.h
//The 1Ring structure of each vertex, similar to the laplacian coordinate, 
//but we use the ARC deformation gradient of RODs for the dynamics

#ifndef _INC_VERTEXRING_ELM_2009_01_H
#define _INC_VERTEXRING_ELM_2009_01_H

#include <memmgr.h>
#include "element_gyrod.h"
#include "element_simp_shear.h"
#include "gspring_element.h"

//#define CVertexRingElementBase CShearElement 
//#define CVertexRingElementBase CGyrodElement
#define CVertexRingElementBase CSimpShearElement

const Vector4d IdentityQuat(0, 0, 0, 1);

class CVertexRingElement
{
public:
	//the 1-ring node class
	class CVertexRingNode: public CVertexRingElementBase
	{
	public:
		int m_nVertexID;					//ID of the vertex
		CMeMaterialProperty *m_pMaterial;	//material property for element
		Vector3d m_lpos0;					//position in local, in initial time step
		double m_weight;					//weight for the node

		CVertexRingNode(void): CVertexRingElementBase()
		{
			m_nVertexID = 0;
			m_pMaterial = NULL;
			m_weight = 0;
		}
		
		CVertexRingNode(const int id, const CMeMaterialProperty *pmaterial, const Vector3d &pos, const double& vol, const double& w): 
			CVertexRingElementBase(*pmaterial, pos, Vector3d(0, 0, 0), vol)
		{
			m_nVertexID = id;
			m_pMaterial = (CMeMaterialProperty *)pmaterial;
			m_lpos0 = pos;
			m_weight = w;
		}
	};

	double3x3 m_R;			//rotation matrix for current time step
	double3x3 m_R0;			//rotation matrix for previous time
	unsigned int m_rotTime;	//rotation matrix computation time
	Vector4d m_quat;		//quaternion rep. of m_R, use vector4d to remove name conflict
	Vector4d m_quat0;		//quaternion rep. of m_R0
	Vector3d m_normal;		//vertex normal, in world space

	//the element data members
	CVertexRingNode* m_pVertexRingNode;
	int m_nVertexID;		//center node id
	int m_nv;				//number of 1-Ring neighbors

private:
	void _getLoacalVertexPositions(const int stride, const Vector3d *parray, Vector3d *pOneRingNodePos) const;

	void _compute1RingForceStatic(
		const Vector3d ringnodepos[],		//1ring nodes' current positions, in the local coordinate
		Vector3d F[],						//force array
		double3x3 *ppJacobian[]);			//jacobian array

	void _compute1RingForceDynamic(
		const int needCompRotation, 
		const Vector3d ringnodepos[],		//1ring nodes' current positions, in local coordiante
		Vector3d F[],						//force array
		double3x3 *ppJacobian[]);			//jacobian array

	void _computeRotationMatrix(const Vector3d* vertexpos, double3x3& A, double3x3& rot, int& rank) const;

	bool _updateRotationMatrixSolidOrShell(const int isshell, const Vector3d* vertexpos);

public:          
	void init(
		const int vid, const Vector3d& pos, 
		const int nRod, const int pRingNodeID[], const CMeMaterialPropertyPtr pMaterial[], 
		const Vector3d pRingNodePos[], const double edgevol[], 
		CMemoryMgr &mgr);

	CVertexRingElement(
		const int vid, const Vector3d& pos, 
		const int nRod, const int *pRingNodeID, const CMeMaterialPropertyPtr pMaterial[], 
		const Vector3d pRingNodePos[], const double edgevol[], 
		CMemoryMgr &mgr)
	{
		init(vid, pos, nRod, pRingNodeID, pMaterial, pRingNodePos, edgevol, mgr);
	}

	CVertexRingElement(void)
	{
		m_nVertexID = -1;	
		m_nv = 0;
		m_pVertexRingNode = NULL;
		m_R.setIdentityMatrix();
		m_R0.setIdentityMatrix();
		m_quat = IdentityQuat;
		m_quat0 = IdentityQuat;
	}

	~CVertexRingElement(void){}

	inline void getElementNodeBuffer(int *buffer) const
	{
		for (int i=0; i<m_nv; i++){
			buffer[i] = m_pVertexRingNode[i].m_nVertexID;
		}
	}

	inline int getCenterVertexID(void) const
	{
		return m_nVertexID;	
	}

	inline const int getRodNumber(void) const
	{
		return m_nv;					
	}

	//compute tensile+bending force for a shear element in triangle mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	void computeForce(
		const int needCompRotation, 
		const int isStatic,					//static simulation or dynamic simu	
		const Vector3d pvert[],				//nodes' current positions
		const int stride,					//consider nodes are packed in an array which are not continious
		const CMeMaterialProperty* pmtl,	//material
		Vector3d F[],						//force array
		double3x3 *ppJacobian[]);			//jacobian array

	//compute the symbolic Jacobian with Lambda, Mu
	void computeJacobianSymbolic(
		const CMeMaterialProperty* pmtl,
		double3x3 pJacobianLambda[], double3x3 pJacobianMu[]);

	bool computeRotationMatrix(const Vector3d* vertexpos, double3x3& rot);

	//implicitly update m_R and m_R0
	bool computeRotationQuaternionSolidOrShell(
		const int isshell, const Vector3d* vertexpos, const int stride);

	int findCommonNeighbors(const CVertexRingElement& e0, const int noinclude_id, int buffer[]);

	void addOneNeighorNodeInCaseOnlyTwoNeighbors(const Vector3d* pVertex, const int vid2, CMemoryMgr &mgr);

};


void computeLeastSquareMatrix(
	const int nv, 
	const double *_P, const int strideP, 
	const double *_Q, const int strideQ, 
	const double *_Weight, const int strideW, 
	double3x3 &A);

#endif