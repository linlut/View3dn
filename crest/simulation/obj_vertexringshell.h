//FILE: obj_vertexringshell.h
#ifndef _INC_OBJVERTEXRINGSHELL_ZN2009_H
#define _INC_OBJVERTEXRINGSHELL_ZN2009_H

#include <assert.h>
#include <vectorall.h>
#include "obj_vertexring.h"


class CSimuVertexRingShell: public CSimuVertexRingObj
{
protected:
	int m_nElementType;			//Surface element type
	int* m_pElement;			//surface element array
	int m_nSurfaceElement;		//# of surface element

private:
	void _initClassVars(void)
	{
		m_nElementType = 0;
		m_pElement = NULL;
		m_nSurfaceElement = 0;
	}

	void _init(
		const Vector3d *pVertex,			//vertex buffer
		const int springtype,				//there are several types to build the stiff K
		const int *pElement,				//element buffer
		const int nv_per_elm,				//vertex # in each element, tet=3, hexahedron=4
		const int nelm,						//element information
		const double& thickness,			//shell thickness
		const CMeMaterialProperty &mtl);

	//compute elastic forces
	void _computeElasticForces(const bool needjacobian);

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex, const int *pElement, 
		const int nv_per_elm, const int nelm, const double &thickness, const double &rho);

	bool _exportMeshTXT(const char *fname);

	bool _exportMeshPLT(FILE *fp);

public:
	CSimuVertexRingShell(
		const int springtype,						//methods for edge mass, 0: vanGalder, 1: Lloyd, else: Lloyd
		const Vector3d *pVertex,					//vertex array 
		const int nv,								//vertex array length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement,						//element buffer
		const int nv_per_elm,						//vertex # in each element, tet=3, hexahedron=4
		const int nelm,								//element information
		const double& thickness,					//shell thickness
		const CMeMaterialProperty &mtl)				//material of the solid
		:CSimuVertexRingObj(pVertex, nv, init_velocity)
	{
		_init(pVertex, springtype, pElement, nv_per_elm, nelm, thickness, mtl);
	}

	virtual ~CSimuVertexRingShell(void);

	virtual bool exportMeshPlt(FILE *fp)
	{
		bool r;
		if (m_pElement==NULL)
			r = CSimuVertexRingObj::exportMeshPlt(fp);
		else 
			r = this->_exportMeshPLT(fp);
		return r;
	}

	virtual void updateRotationQuaternionForAllElements(const unsigned int tm, const bool needQuat);

	virtual int exportOBJFileObject(const int stepid, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const;

	virtual void computeStiffnessMatrix(void)
	{
		assert(0);
	}

};


#endif