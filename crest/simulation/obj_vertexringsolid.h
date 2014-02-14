//FILE: obj_vertexringsolid.h
#ifndef _INC_OBJVERTEXRINGSOLID_ZN2009_H
#define _INC_OBJVERTEXRINGSOLID_ZN2009_H

#include <assert.h>
#include <vectorall.h>
#include "obj_vertexring.h"
#include "obj_spring.h"


class CSimuVertexRingSolid: public CSimuVertexRingObj
{
protected:
	int m_nElementType;
	int* m_pElement;

	int *m_pSurfacePoly;	//surface polygon array
	int m_nSurfaceType;		//=3 or 4, number of nodes per surface element
	int m_nSurfacePoly;		//# of surface polygon

private:
	void _initClassVars(void);

	void _init(
		const Vector3d *pVertex,			//vertex buffer
		const int springtype,				//there are several types to build the stiff K
		const int *pElement,				//element buffer
		const int nv_per_elm,				//vertex # in each element, tet=3, hexahedron=4
		const int nelm,						//element information
		const CMeMaterialProperty &mtl);

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex, const int *pElement, 
		const int nv_per_elm, const int nelm, const double &rho);

	bool _exportMeshTXT(const char *fname);

	bool _exportMeshPLT(FILE *fp);

public:
	CSimuVertexRingSolid(
		const int springtype,						//methods: 0: vanGalder, 1: Lloyd, else: Lloyd
		const Vector3d *pVertex,					//vertex array 
		const int nv,								//vertex array length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement,						//element buffer
		const int nv_per_elm,						//vertex # in each element, tet=3, hexahedron=4
		const int nelm,								//element information
		const CMeMaterialProperty &mtl)				//material of the solid
		:CSimuVertexRingObj(pVertex, nv, init_velocity)
	{
		_init(pVertex, springtype, pElement, nv_per_elm, nelm, mtl);
	}

	void setBoundarySurface(
		const int* pSurfacePoly,					//surface polygon array
		const int nSurfaceType,						//=3 or 4, number of nodes per surface element
		const int nSurfacePoly);					//# of surface polygon

	CSimuVertexRingSolid(
		const int springtype,						//there are several types to build the edge mass, 0: vanGalder, 1: Lloyd, else: Lloyd
		const Vector3d *pVertex,					//vertex array 
		const int nv,								//vertex array length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement,						//element buffer
		const int nv_per_elm,						//vertex # in each element, tet=3, hexahedron=4
		const int nelm,								//element information
		const int* pSurfacePoly,					//surface polygon array
		const int nSurfaceType,						//=3 or 4, number of nodes per surface element
		const int nSurfacePoly,						//# of surface polygon
		const CMeMaterialProperty &mtl)				//material of the solid
		:CSimuVertexRingObj(pVertex, nv, init_velocity)
	{
		_init(pVertex, springtype, pElement, nv_per_elm, nelm, mtl);
		setBoundarySurface(pSurfacePoly, nSurfaceType, nSurfacePoly);
	}

	virtual ~CSimuVertexRingSolid(void);

	virtual int exportOBJFileObject(const int stepid, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const;

	virtual bool exportMeshPlt(FILE *fp)
	{
		bool r;
		if (m_pSurfacePoly==NULL)
			r = CSimuVertexRingObj::exportMeshPlt(fp);
		else 
			r = this->_exportMeshPLT(fp);
		return r;
	}

};


extern void ConvertEdgeMapToEdgeInput(
	const Vector3d *pVertex, const CMeMaterialProperty *pmat, CEdgeMap& edgemap, //input
	vector<CSimuEdgeInput> &edgeinput);		//output


#endif