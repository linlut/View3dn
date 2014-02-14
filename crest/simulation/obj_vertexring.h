//FILE: obj_vertexring.h
#ifndef _INC_VERTEXRINGOBJ_ZN200901_H
#define _INC_VERTEXRINGOBJ_ZN200901_H

#include <assert.h>
#include <vectorall.h>
#include "simu_entity.h"
#include "vertexring_element.h"

class CSparseMatrix33;

class CSimuEdgeInput
{
public:
	int v0, v1;
	double area;
	double mass;
	CMeMaterialProperty *pMaterial;
	double3x3 mat;					//tempararily, used to store a rotation matrix

public:
	CSimuEdgeInput(){}

	CSimuEdgeInput(
		const int _v0, const int _v1,	//node indices
		const double _area,				//edge corss area
		const double _mass,				//edge mass
		const CMeMaterialProperty *_pm)://material pointer
		v0(_v0), v1(_v1), area(_area), mass(_mass)
	{
		pMaterial = (CMeMaterialProperty*)_pm;
	}
};


typedef CVertexRingElementBase CGyrodElementType;

class CSimuVertexRingObj: public CSimuEntity
{
protected:
	CVertexRingElement *m_pVRingElement;	//array of vextex ring element
	CMemoryMgr m_mgr;  						//memory allocer
	int m_nVRingElementCount;				//array size

private:
	int m_nAlg;								//algorithm selection: 0-fast 1-slower alg.
	int m_nRotationSkipStep;				//corotational formulation, every n steps to compute rot once

	//==============Used for the new alg.========================
	int m_nEdge;							//#edges
	CSimuEdgeInput *m_pEdge;				//edges	
	CGyrodElementType *m_pGyrod;			//array same as the edge

	//==============Used for the new alg.========================
	//compute elastic forces
	void _computeElasticForces(const unsigned int timeid, const bool isStatic, const bool needjacobian);
	//a new algorithm researching
	void _computeElasticForcesNew(const unsigned int timeid, const bool isStatic, const bool needjacobian);

	bool _exportMeshTXT(const char *fname);

	void _initVars(void); //for var init shared in both constructors

public:
	CSimuVertexRingObj(
		const Vector3d *pVertex, const int nv,	//vertex array and length
		const Vector3d &init_velocity):			//vertex velocity
		m_mgr(nv/10+40),
		CSimuEntity(pVertex, nv, init_velocity, NULL)
	{
		_initVars();
	}

	void init(
		const CSimuEdgeInput *pEdge,			//edges	
		const int nedge,						//# edges
		const CMeMaterialProperty& mtl			//material pointer
		);						

	CSimuVertexRingObj(
		const Vector3d *pVertex, const int nv,	//vertex array and length
		const Vector3d &init_velocity,			//vertex velocity
		CSimuEdgeInput *pEdge,					//edges	
		const int nedge,						//# edges
		const CMeMaterialProperty& mtl			//material pointer
		) : 
		CSimuEntity(pVertex, nv, init_velocity, &mtl), 
		m_mgr(nv/10+12)
	{
		_initVars();
		init(pEdge, nedge, mtl);
	}

	virtual ~CSimuVertexRingObj(void);

	inline int getRodCount(void) const
	{
		int n = 0;
		for (int i=0; i<m_nVRingElementCount; i++) 
			n += m_pVRingElement[i].getRodNumber();
		return n;
	}

	inline void setAlgorithm(const int i)
	{
		m_nAlg = i;
	}

	inline void setRotationSkipStep(const int i)
	{
		m_nRotationSkipStep = i;
	}

	//find an element
	CGyrodElementType* findGyrodElementByNodeID(const int v0, const int v1, int &index, int &nv0, int &nv1); 

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false);

	//update necessary state variables after time integration
	virtual void updateState(void){}

	virtual bool exportMeshPlt(FILE *fp);

	virtual void exportMesh(const int count, const char *name, const char *ext);

	virtual void updateRotationQuaternionForAllElements(const unsigned int tm, const bool needQuat);

	virtual void computeStiffnessMatrix(void);

	virtual void computeStiffnessSymbolic(CSparseMatrix33 *l, CSparseMatrix33 *m);

};


#endif