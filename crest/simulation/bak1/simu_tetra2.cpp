//FILE: simu_thinshell.cpp

#include <geomath.h>
#include <view3dn/edgetable.h>
#include "simu_tetra2.h"


CSimuTetra2::~CSimuTetra2(void)
{
	SafeDeleteArray(m_pTetra2Element);
}

void CSimuTetra2::_computeNodeMass(
	const Vector3d *pVertex, const int *pElement, const int nelm, const double &rho)
{
	ComputeNodeMassTet(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
}


CSimuTetra2::CSimuTetra2(
		const Vector3d *pVertex, const int nv,		//vertex array and length	
		const Vector3d &init_velocity,				//initial vertex velocity
		const int *pElement, const int ntetelm,		//element array
		const CMeMaterialProperty& mtl				//material property
		):
	CSimuEntity(pVertex, nv, init_velocity, &mtl)
{
	//alloc buffer
	m_nElementCount = ntetelm;	
	m_pTetra2Element = new CTetra2Element[m_nElementCount];
	assert(m_pTetra2Element!=0);

	//first, need to decide mass
	_computeNodeMass(pVertex, pElement, ntetelm, m_mtl.m_rho);

	//then, init each tetra2 element
	const Vector4i *pquad = (const Vector4i*)pElement;
	for (int i=0; i<m_nElementCount; i++){
		CTetra2Element *p = &m_pTetra2Element[i];
		const Vector4i quad = pquad[i];
		const Vector3d& p0 = pVertex[quad.x];
		const Vector3d& p1 = pVertex[quad.y];
		const Vector3d& p2 = pVertex[quad.z];
		const Vector3d& p3 = pVertex[quad.w];
		p->init(&quad.x, p0, p1, p2, p3);
	}

	printf("There are %d shell elements generated.\n", m_nElementCount);

}


void CSimuTetra2::updateState(void)
{
	for (int i=0; i<m_nElementCount; i++){
		m_pTetra2Element[i].updateState();
	}
}


void CSimuTetra2::_computeElasticForces(void)
{
	Vector3d force[4];
	for (int i=0; i<m_nElementCount; i++){
		CTetra2Element &E = m_pTetra2Element[i];
		const int* pNodeBuffer = E.getElementNodeBuffer();
		const int ia = pNodeBuffer[0];
		const int ib = pNodeBuffer[1];
		const int ic = pNodeBuffer[2];
		const int id = pNodeBuffer[3];
		Vector3d& p0 = m_pVertInfo[ia].m_pos;
		Vector3d& p1 = m_pVertInfo[ib].m_pos;
		Vector3d& p2 = m_pVertInfo[ic].m_pos;
		Vector3d& p3 = m_pVertInfo[id].m_pos;
		E.computeForce(p0, p1, p2, p3, m_mtl, force);
		m_pVertInfo[ia].m_force += force[0];
		m_pVertInfo[ib].m_force += force[1];
		m_pVertInfo[ic].m_force += force[2];
		m_pVertInfo[id].m_force += force[3];
	}
}

//compute the acceleration vector for the current location 
void CSimuTetra2::computeAcceleration(const unsigned int timeid, const bool needJacobian)
{
	//init forces as zero and add gravity load
	initForceAndApplyGravity();

	//add elastic forces
	_computeElasticForces();

	//add collison forces

	//compute acc
	for (int i=0; i<m_nVertexCount; i++){
		VertexInfo *node = &m_pVertInfo[i];
		Vector3d &a = node->m_acc;
		Vector3d &f = node->m_force;
		double invmass = 1.0/node->m_mass;
		a = f * invmass;
	}
}


//export mesh
bool CSimuTetra2::_exportMeshPLT(const char *fname)
{
	int i;
	char PLYTYPESTR[2][16]={"TRIANGLE", "QUADRILATERAL"};
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return false;
	//vertex array
	fprintf(fp, "TITLE = SHELLSIMULATION\n");
	fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\"\n");
	fprintf(fp, "ZONE N = %d  E = %d F = FEPOINT, ET = %s\n", m_nVertexCount, m_nElementCount*4, PLYTYPESTR[0]);
	for (i=0; i<m_nVertexCount; i++){
        const Vector3d p= m_pVertInfo[i].m_pos;
		fprintf(fp, "%.12lG %.12lG %.12lG\n", p.x, p.y, p.z);
    }
	//mesh connectivity
    for (i=0; i<m_nElementCount; i++){
		const int *pnodeid = m_pTetra2Element[i].getElementNodeBuffer();
		int x = pnodeid[0];
		int y = pnodeid[1];
		int z = pnodeid[2];
		int w = pnodeid[3];
		x++; y++; z++; w++; 
		fprintf(fp, "%d %d %d\n", x, y, z);
		fprintf(fp, "%d %d %d\n", x, y, w);
		fprintf(fp, "%d %d %d\n", y, z, w);
		fprintf(fp, "%d %d %d\n", x, z, w);
	}
	fclose(fp);
	return 1;
}


void CSimuTetra2::exportMesh(const int count, const char *fname, const char *ext)
{
	char ffname[200];
	sprintf(ffname, "shell%d.plt", count);

	_exportMeshPLT(ffname);
}
