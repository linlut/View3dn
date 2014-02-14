//FILE: simu_thinshell2.cpp
#include <geomath.h>
#include <view3dn/edgetable.h>
#include "simu_thinshell2.h"


void CSimuThinshell2::_find1Ring4TriMesh(
	CThinshell2Element *pshell, const Vector3i *surf, const int nelm)
{
	for (int i=0; i<nelm; i++){
		const Vector3i& tri = surf[i];
		{	const int ii = tri.x;
			int &c = pshell[ii].m_n1RingPoly; 
			pshell[ii].m_n1RingPolyID[c] = i; c++; 
		}
		{	const int ii = tri.y;
			int &c = pshell[ii].m_n1RingPoly; 
			pshell[ii].m_n1RingPolyID[c] = i; c++; 
		}
		{	const int ii = tri.z;
			int &c = pshell[ii].m_n1RingPoly; 
			pshell[ii].m_n1RingPolyID[c] = i; c++; 
		}
	}
}

void CSimuThinshell2::_find1Ring4QuadMesh(
	CThinshell2Element *pshell, const Vector4i *surf, const int nelm)
{
	for (int i=0; i<nelm; i++){
		const Vector4i& tri = surf[i];
		{	const int ii = tri.x;
			int &c = pshell[ii].m_n1RingPoly; 
			pshell[ii].m_n1RingPolyID[c] = i; c++; 
		}
		{	const int ii = tri.y;
			int &c = pshell[ii].m_n1RingPoly; 
			pshell[ii].m_n1RingPolyID[c] = i; c++; 
		}
		{	const int ii = tri.z;
			int &c = pshell[ii].m_n1RingPoly; 
			pshell[ii].m_n1RingPolyID[c] = i; c++; 
		}
		{	const int ii = tri.w;
			int &c = pshell[ii].m_n1RingPoly; 
			pshell[ii].m_n1RingPolyID[c] = i; c++; 
		}
	}
}

CSimuThinshell2::CSimuThinshell2(
		const Vector3d *pVertex, const int nv,		//vertex array and length	
		const Vector3d &init_velocity,				//initial vertex velocity
		const int *pElement, const int nv_per_elm, const int nelm,	//surf. elm array, not shell elm
		const double &shell_thickness,				//shell thinkness, seems not useful
		const CMeMaterialProperty& mtl,				//material property
		const int isStatic							//static or dynamic simulation
		):
	CSimuShell(pVertex, nv, init_velocity, pElement, nv_per_elm, nelm,
		shell_thickness, mtl, isStatic)
{
	//first, need to decide mass
	const double rho = mtl.getDensity();
	computeNodeMass(pVertex, rho);
	const int stride=sizeof(Vector3d);
	computeSurfaceAreaWeightedNormal(pVertex, stride);

	//then, for each vertex build a thinshell2 element
	m_nElementCount = nv;		
	m_pThinshellElement = new CThinshell2Element[m_nElementCount];
	assert(m_pThinshellElement!=NULL);

	//build one ring fan, store in m_nNodeID buffer of CThinshell2Element struct.
	if (nv_per_elm==3)
		_find1Ring4TriMesh(m_pThinshellElement, (const Vector3i *)pElement, nelm);
	else{
		ASSERT0(nv_per_elm==4);
		_find1Ring4QuadMesh(m_pThinshellElement, (const Vector4i *)pElement, nelm);
	}

	//traverse all the edges to init the shell elements
	int polyid[100];
	for (int i=0; i<nv; i++){
		const int nodeid = i;
		CThinshell2Element& e = m_pThinshellElement[i];
		const int nPoly = e.m_n1RingPoly;
		for (int j=0; j<nPoly; j++) polyid[j]=e.m_n1RingPolyID[j];
		e.init(m_static, mtl, nodeid, pVertex, m_pSurfaceNorm, pElement, nv_per_elm, polyid, nPoly, shell_thickness);
	}

	printf("Vertex#: %d, polygon#: %d, shell element#:%d.\n", nv, nelm, m_nElementCount);
}


void CSimuThinshell2::_computeElasticForces(const bool needjacobian)
{
	const int NV = MAX_NODE_VALENCE+1;
	Vector3d force[NV];
	double3x3 jacobian[NV], *ppjac[NV];
	if (!needjacobian){
		for (int i=0; i<NV; i++) ppjac[i] = NULL;
	}
	else{
		for (int i=0; i<NV; i++) ppjac[i] = &jacobian[i];
	}

	//compute surface normal first
	const Vector3d *pVertex = &m_pVertInfo[0].m_pos;
	const int stride = sizeof(CSimuEntity::VertexInfo);
	computeSurfaceAreaWeightedNormal(pVertex, stride);

	//for all the elements, compute forces
	for (int i=0; i<m_nElementCount; i++){
		CThinshell2Element &E = m_pThinshellElement[i];
		const int &nRod = E.m_nRod;
		const int &v0 = E.m_nCenterID;	
		const int* pNodeBuffer = E.getElementNodeBuffer();
		E.computeForce(m_static, pVertex, stride, m_pSurfaceNorm, m_mtl, force, ppjac);
		m_pVertInfo[v0].m_force += force[0];
		for (int j=0; j<nRod; j++){
			const int vj = pNodeBuffer[j]; 
			m_pVertInfo[vj].m_force += force[j+1];
		}
		if (needjacobian){
			for (int j=0; j<nRod; j++){
				const int vj = pNodeBuffer[j]; 
				saveVertSitffMatrixIntoSparseMatrix(vj, v0, jacobian[j+1]);
			}
		}
	}//end for
}

//compute the acceleration vector for the current location 
void CSimuThinshell2::computeAcceleration(const unsigned int timeid, const bool needJacobian)
{
	//init forces as zero and add gravity load
	initForceAndApplyGravity();

	//add elastic forces
	_computeElasticForces(needJacobian);

	//compute acc
	for (int i=0; i<m_nVertexCount; i++){
		VertexInfo *node = &m_pVertInfo[i];
		Vector3d &a = node->m_acc;
		Vector3d &f = node->m_force;
		double invmass = 1.0/node->m_mass;
		a = f * invmass;
	}
}

static int _totalPolygon(const CThinshell2Element *pelm, const int ne)
{
	int c = 0;
	for (int i=0; i<ne; i++)
		c+=pelm[i].get1RingPolygonNumber();
	return c;
}

static int test_func(void)
{
	/*
	CMeMaterialProperty mtl;
	mtl.setYoung(1e5);
	const int nv=6;
	Vector3d pVertex[nv]={
		Vector3d(0, 0, 0),
		Vector3d(1, 0, 0),
		Vector3d(0, 1, 0),
		Vector3d(1, 1, 0),
		Vector3d(0.5, 1.5, 0),
		Vector3d(0.5, -0.5, 0)
	};
		
	Vector3d init_velocity(0,0,1);
	const int nelm=4;
	Vector3i element[nelm]={
		Vector3i(0, 1, 3), 
		Vector3i(0, 3, 2),
		Vector3i(2, 3, 4),
		Vector3i(0, 5, 1)
	};
	const int nv_per_elm=3;
	const double shell_thickness=0.001;

	CSimuThinshell2 s(
		pVertex, nv,					//vertex array and length
		init_velocity,					//vertex velocity
		&element[0].x, nv_per_elm, nelm,//element information
		shell_thickness,
		mtl,							//material property
		0
		);

	//set constraints
	CVertexConstraints constraints;
	constraints.setFixedPositionXYZ();
	s.setConstraints(0, constraints);
	s.setConstraints(3, constraints);
	s.setGravityAccelertion(Vector3d(0,0,-10));

	//velocity verlet integration method
	const double dt=0.001;		//1MS
	for (int i=0; i<20; i++){
		//s.dumpVertexPosition(4, i, stdout);
		VerletIntegration(s, i, dt);		
	}
*/
	return 1;
}

static int test = test_func();
