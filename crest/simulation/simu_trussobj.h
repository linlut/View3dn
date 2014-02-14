//FILE: simu_trussobj.h
#ifndef _INC_TRUSSOBJ_ZN2008_H
#define _INC_TRUSSOBJ_ZN2008_H

#include <assert.h>
#include <vectorall.h>
#include "truss_element.h"
#include "simu_entity.h"


class CSimuTrussObj: public CSimuEntity
{
protected:
	CTrussElement *m_pTrussElement;		
	double m_K;							//stiff ratio
	double m_Kd;						//damping stiff ratio

private:
	//compute elastic forces
	void _computeElasticForces(const bool needjacobian);

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex, const int isSolid, const int *pElement, const int nv_per_elm, const int nelm, const double &thickness, const double &rho);

	bool _exportMeshTXT(const char *fname);

	bool _exportMeshPLT(const char *fname);

public:
	CSimuTrussObj(
	    const int springtype,						//there are several types to build the stiff K
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int issolid,							//solid or surface 1: solid, 0: surface
		const int *pElement, const int nv_per_elm, const int nelm,	//element information
		const double thickness,
		const CMeMaterialProperty &mtl
		);

	virtual ~CSimuTrussObj(void);

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false);

	//update necessary state variables after time integration
	virtual void updateState(void){}

	virtual void exportMesh(const int count, const char *name, const char *ext);
};


#endif