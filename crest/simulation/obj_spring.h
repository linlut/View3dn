//FILE: simu_springobj.h
#ifndef _INC_SIMUSPRINGOBJ_ZN2008_H
#define _INC_SIMUSPRINGOBJ_ZN2008_H

#include <map>
#include <assert.h>
#include <vectorall.h>
#include "spring_element.h"
#include "simu_entity.h"


typedef CEdgeAuxStruct CSpringAuxStruct;
typedef map<INT64KEY, CSpringAuxStruct> CEdgeMap;

extern void buildEdgeMapFromSolidElements(
	const int springtype,	//0: van Gedar 94?, 1: Lloyd 2008 TVCG
	const Vector3d *pVertex, const int elmtype, const int *pElement, const int nelm, const double rho, 
	CEdgeMap& edgemap);


class CSimuSpringObj: public CSimuEntity
{
protected:
	CSpringElement *m_pSpringElement;		
	int *m_pSurfacePoly;	//surface polygon array
	int m_nSurfaceType;		//=3 or 4, number of nodes per surface element
	int m_nSurfacePoly;		//# of surface polygon

	//volume preservation need
	int m_nElementType;		//the input element type
	int m_nElement;			//number of input elements
	int *m_pTet;			//tet mesh
	double *m_pTetVolume;	//volume

private:	
	//compute elastic forces
	void _computeElasticForces(const bool needjacobian);

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex, const int isSolid, 
		const int *pElement, const int nv_per_elm, const int nelm, 
		const double &thickness, const double &rho);

	bool _exportMeshTXT(const char *fname);

	bool _exportMeshPLTLine(FILE *fp);

	bool _exportMeshPLTSurface(FILE *fp);

public:
	CSimuSpringObj(
	    const int springtype,						//there are several types to build the stiff K
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int issolid,							//solid or surface 1: solid, 0: surface
		const int *pElement,						//solid element information
		const int nv_per_elm, 
		const int nelm,	
		const double thickness,
		const CMeMaterialProperty &mtl
		);

	virtual ~CSimuSpringObj(void);

	void setBoundarySurface(
		const int* pSurfacePoly,					//surface polygon array
		const int nSurfaceType,						//=3 or 4, number of nodes per surface element
		const int nSurfacePoly);					//# of surface polygon

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJac=false);

	//update necessary state variables after time integration
	virtual void updateState(void){}

	virtual void exportMesh(const int count, const char *name, const char *ext);

	virtual bool exportMeshPlt(FILE *fp);

	virtual void computeStiffnessMatrix(void)
	{
		assert(0);
	}

};


#endif