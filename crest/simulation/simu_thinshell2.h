//FILE: simu_thinshell2.h
#ifndef _INC_SIMU_ROTINVARIANT_THINSHELL_ZN2008_H
#define _INC_SIMU_ROTINVARIANT_THINSHELL_ZN2008_H

#include <assert.h>
#include <vectorall.h>
#include "thinshell2_element.h"
#include "simu_shell.h"


class CSimuThinshell2: public CSimuShell
{
protected:
	CThinshell2Element *m_pThinshellElement;//thinshell element array
											//# of thinshell elm. can be found in m_nElementCount of CSimuEntity
											//NOTE that it is not the input triangle count!!!

private:
	void _find1Ring4TriMesh(CThinshell2Element *pshell, const Vector3i *surf, const int nelm);

	void _find1Ring4QuadMesh(CThinshell2Element *pshell, const Vector4i *surf, const int nelm);

	//compute elastic forces
	void _computeElasticForces(const bool needjacobian);

public:
	CSimuThinshell2(
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement, const int nv_per_elm, const int nelm,	//element information
		const double &shell_thickness,
		const CMeMaterialProperty& mtl,				//material property
		const int isStatic=0						//static or dynamic simulation
		);

	virtual ~CSimuThinshell2(void)
	{
		SafeDeleteArray(m_pThinshellElement);
		m_pThinshellElement = NULL;
	}

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false);

	//update necessary state variables after time integration
	virtual void updateState(void){}

};


#endif