//FILE: obj_hexshell.h
#ifndef _INC_HEXSHELLOBJ_ZN2009NOV_H
#define _INC_HEXSHELLOBJ_ZN2009NOV_H

#include "element_hexshell.h"
#include "simu_entity.h"


class CSimuHexshellObj: public CSimuEntity
{
protected:
	CHexshellElement *m_pElement;		
	int *m_pSurfacePoly;	//surface polygon array
	int m_nSurfacePoly;		//# of surface polygon

private:	
	//compute elastic forces
	void _computeElasticForces(const unsigned int timeid, const bool needjacobian);

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex, const int *pElement, const int nelm, const double &rho);

	bool _exportMeshTXT(const char *fname);

	bool _exportMeshPLTLine(FILE *fp);

	bool _exportMeshPLTSurface(FILE *fp);

public:
	CSimuHexshellObj(
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement,						//solid element information
		const int nelm,	
		const CMeMaterialProperty &mtl
		);

	virtual ~CSimuHexshellObj(void);

	void setBoundarySurface(const int* pSurfacePoly, const int nSurfacePoly);	

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJac=false);

	//update necessary state variables after time integration
	virtual void updateState(void){}

	virtual void exportMesh(const int count, const char *name, const char *ext);

	virtual bool exportMeshPlt(FILE *fp);

	virtual void computeStiffnessMatrix(void);

};


#endif