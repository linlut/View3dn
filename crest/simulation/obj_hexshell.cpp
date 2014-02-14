//FILE: simu_springobj.cpp
#include <geomath.h>
#include "simu_shell.h"
#include "obj_hexshell.h"
#include "zsparse_matrix.h"
#include "time_integration.h"


#define ELEMENT_T CHexshellElement

CSimuHexshellObj::~CSimuHexshellObj(void)
{
	SafeDeleteArray(m_pElement);
}


void CSimuHexshellObj::_computeNodeMass(
	const Vector3d *pVertex, const int *pElement, const int nelm, const double &rho)
{
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	//Hexahedral mesh
	ComputeNodeMassHex(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
}


CSimuHexshellObj::CSimuHexshellObj(
	const Vector3d *pVertex, const int nv,		//vertex array and length	
	const Vector3d &init_velocity,				//initial vertex velocity
	const int *pElement, const int nelm,		//element array
	const CMeMaterialProperty &_mtl)
	:CSimuEntity(pVertex, nv, init_velocity, &_mtl)
{
	m_pSurfacePoly=NULL;
	m_nSurfacePoly=0;		//# of surface polygon
	m_bHasConstantStiffness = ELEMENT_T::hasConstantStiffness();

	//first, need to decide mass
	m_mtl = _mtl;
	_computeNodeMass(pVertex, pElement, nelm, m_mtl.getDensity());

	//allocate the buffer and init
	m_nElementCount = nelm;
	m_pElement = new ELEMENT_T[m_nElementCount];
	assert(m_pElement!=NULL);
	const Vector8i* pHex = (const Vector8i*)pElement;
	for (int i=0; i<nelm; i++){
		const Vector8i* a = &pHex[i];
		const Vector3d& p0 = pVertex[a->x];
		const Vector3d& p1 = pVertex[a->y];
		const Vector3d& p2 = pVertex[a->z];
		const Vector3d& p3 = pVertex[a->w];
		const Vector3d& p4 = pVertex[a->x1];
		const Vector3d& p5 = pVertex[a->y1];
		const Vector3d& p6 = pVertex[a->z1];
		const Vector3d& p7 = pVertex[a->w1];
		m_pElement[i].init(m_mtl, &a->x, p0, p1, p2, p3, p4, p5, p6, p7);
	}
}


void CSimuHexshellObj::setBoundarySurface(
	const int* pSurfacePoly,					//surface polygon array
	const int nSurfacePoly)						//# of surface polygon
{
	const int nSurfaceType = 4;
	SafeDeleteArray(m_pSurfacePoly);
	const int nsize = nSurfaceType * nSurfacePoly;
	m_pSurfacePoly = new int [nsize];
	assert(m_pSurfacePoly!=NULL);
	for (int i=0; i<nsize; i++) m_pSurfacePoly[i]=pSurfacePoly[i];
	m_nSurfacePoly = nSurfacePoly;
}


void CSimuHexshellObj::_computeElasticForces(const unsigned int timeid, const bool needjacobian)
{
	const int N=39;
	Vector3d F[8];
	double3x3 jacobian[N], *pjac[N];
	if (needjacobian){
		for (int i=0; i<N; i++)
			jacobian[i].setZeroMatrix(), pjac[i] = &jacobian[i];
	}
	else
		for (int i=0; i<N; i++) pjac[i] = NULL;
	CSparseMatrix33 *pSM = m_pOdeIntegrator->getSparseMatrix();

	for (int i=0; i<m_nElementCount; i++){
		ELEMENT_T &E = m_pElement[i];
		const int* pNodeBuffer = E.getElementNodeBuffer();
		const int i0 = pNodeBuffer[0];
		const int i1 = pNodeBuffer[1];
		const int i2 = pNodeBuffer[2];
		const int i3 = pNodeBuffer[3];
		const int i4 = pNodeBuffer[4];
		const int i5 = pNodeBuffer[5];
		const int i6 = pNodeBuffer[6];
		const int i7 = pNodeBuffer[7];
		const Vector3d& p0 = m_pVertInfo[i0].m_pos;
		const Vector3d& p1 = m_pVertInfo[i1].m_pos;
		const Vector3d& p2 = m_pVertInfo[i2].m_pos;
		const Vector3d& p3 = m_pVertInfo[i3].m_pos;
		const Vector3d& p4 = m_pVertInfo[i4].m_pos;
		const Vector3d& p5 = m_pVertInfo[i5].m_pos;
		const Vector3d& p6 = m_pVertInfo[i6].m_pos;
		const Vector3d& p7 = m_pVertInfo[i7].m_pos;
		E.computeForce(timeid, 0, p0, p1, p2, p3, p4, p5, p6, p7, m_mtl, F, pjac);
		m_pVertInfo[i0].m_force += F[0];
		m_pVertInfo[i1].m_force += F[1];
		m_pVertInfo[i2].m_force += F[2];
		m_pVertInfo[i3].m_force += F[3];
		m_pVertInfo[i4].m_force += F[4];
		m_pVertInfo[i5].m_force += F[5];
		m_pVertInfo[i6].m_force += F[6];
		m_pVertInfo[i7].m_force += F[7];
		if (needjacobian) 
			E.saveStiffness(pSM, jacobian);
	}
}

//compute the acceleration vector for the current location 
void CSimuHexshellObj::computeAcceleration(const unsigned int timeid, const bool needJacobian)
{
	//init forces as zero and add gravity load
	initForceAndApplyGravity();

	//add elastic forces
	_computeElasticForces(timeid, needJacobian);

	//compute acc
	for (int i=0; i<m_nVertexCount; i++){
		VertexInfo *node = &m_pVertInfo[i];
		Vector3d &a = node->m_acc;
		Vector3d &f = node->m_force;
		double invmass = 1.0/node->m_mass;
		a = f * invmass;
	}
}


bool CSimuHexshellObj::_exportMeshPLTSurface(FILE *fp)
{
	return ExportMeshPLTSurface(m_pVertInfo, m_nVertexCount, m_pSurfacePoly, 4, m_nSurfacePoly, fp);
}


void CSimuHexshellObj::exportMesh(const int count, const char *fname, const char *ext)
{
	char ffname[200];
	sprintf(ffname, "spring%d.plt", count);

	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return;
	exportMeshPlt(fp);
	if (fp) fclose(fp);
}


bool CSimuHexshellObj::exportMeshPlt(FILE *fp)
{
	bool r = _exportMeshPLTSurface(fp);
	return r;
}


void CSimuHexshellObj::computeStiffnessMatrix(void)
{
	//check sparse matrix
	CSparseMatrix33 *pmat = this->getOdeIntegrator()->getSparseMatrix();
	if (pmat==NULL) return;

	const bool needjacobian = true;
	const unsigned int timeid = 0;
	_computeElasticForces(timeid, needjacobian);
}
