//FILE: simu_static.cpp

#include <geomath.h>
#include <view3dn/edgetable.h>
#include "simu_staticrod.h"


void ComputeNodeMassForRods(const Vector3d *pVertex, const int nv,
	const Vector2i *pElement, const int nelm, 
	const double &crossarea, const double &rho, 
	CSimuEntity::VertexInfo *m_pVertInfo)
{
	int i;
	for (i=0; i<nv; i++) m_pVertInfo[i].m_mass = 0;
	const double K=crossarea*rho*0.5;
	for (i=0; i<nelm; i++){
		const Vector2i ll = pElement[i];
		const double len = Distance(pVertex[ll.x], pVertex[ll.y]);
		const double mass = len*K;
		m_pVertInfo[ll.x].m_mass += mass;
		m_pVertInfo[ll.y].m_mass += mass;
	}
}



