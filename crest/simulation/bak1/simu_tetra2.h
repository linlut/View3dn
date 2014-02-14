//FILE: simu_tetra2.cpp
#ifndef _INC_SIMUTETRA2_NZ2008_H
#define _INC_SIMUTETRA2_NZ2008_H

#include <assert.h>
#include <vectorall.h>
#include "tetra_element.h"
#include "simu_entity.h"


class CSimuTetra2: public CSimuEntity
{
protected:
	CTetra2Element *m_pTetra2Element;		//tetrahedral element array represnted using shear elm
	int m_nTetra2Element;					//number of thinshell element
											//NOTE that it is not the input triangle count!!!
	CMeMaterialProperty m_mtl;				//Computational mech. based material property

private:
	//compute elastic forces
	void _computeElasticForces(void);

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex,const int *pTetElement, const int ntetelm, const double &rho);

	bool _exportMeshPLT(const char *fname);

public:
	CSimuTetra2(
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pTetElement, const int ntetelm,	//element information
		const CMeMaterialProperty& mtl				//material property
		);

	virtual ~CSimuTetra2(void);

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false);

	//update necessary state variables after time integration
	virtual void updateState(void);

	virtual void exportMesh(const int count, const char *name, const char *ext);

	virtual bool exportMeshPlt(FILE *fp){ return false; }

};


#endif