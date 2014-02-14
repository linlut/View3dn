//FILE: obj_tetfem2.h
//Linear FEM for tet elements

#ifndef _INC_OBJTETFEM2_NZ20090830_H
#define _INC_OBJTETFEM2_NZ20090830_H

//Tet mesh simulation using Green strain tet element
//#include "obj_tetfem.h"
//#include "element_tetfem2.h"
//typedef _CSimuTetra<CTetFem2Element> CSimuTetra2;
//typedef CSimuTetra2 CSimuTetraGreen;


/*
#include <assert.h>
#include <vector>
#include <vectorall.h>
#include "simu_entity.h"
#include "element_tetfem2.h"


class CSimuTetra2: public CSimuEntity
{
protected:
	int m_nIsStatic;						//static, small strain FEM, doesnot need to compute rotation
	CTetFem2Element *m_pTetraElement;		//tetrahedral element array 
	int m_nTetra2Element;					//number of tet element
	int* m_pSurfacePoly;
	int m_nSurfacePoly;

private:
	//compute elastic forces
	void _computeElasticForces(const unsigned int timeid, const bool needjacobian);

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex,const int *pTetElement, const int ntetelm, const double &rho);

	bool _exportMeshPLT(const char *fname);

public:
	CSimuTetra2(
		const int isStatic,
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pTetElement, const int ntetelm,	//element information
		const int *pFace, const int nface,
		const CMeMaterialProperty& mtl				//material property
		);

	virtual ~CSimuTetra2(void);

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false);

	//update necessary state variables after time integration
	virtual void updateState(void);

	virtual void exportMesh(const int count, const char *name, const char *ext);

	virtual bool exportMeshPlt(FILE *fp);

};
*/


#endif
