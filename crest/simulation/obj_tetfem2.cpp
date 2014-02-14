//FILE: obj_tetfem2.cpp

//#include <geomath.h>
//#include "obj_tetfem2.h"


/*
CSimuTetra2::~CSimuTetra2(void)
{
	SafeDeleteArray(m_pTetraElement);
	SafeDeleteArray(m_pSurfacePoly);
}

void CSimuTetra2::_computeNodeMass(
	const Vector3d *pVertex, const int *pElement, const int nelm, const double &rho)
{
	ComputeNodeMassTet(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
}


CSimuTetra2::CSimuTetra2(
		const int isStatic,
		const Vector3d *pVertex, const int nv,		//vertex array and length	
		const Vector3d &init_velocity,				//initial vertex velocity
		const int *pElement, const int ntetelm,		//element array
		const int *pFace, const int nface,
		const CMeMaterialProperty& mtl				//material property
		):
	CSimuEntity(pVertex, nv, init_velocity, &mtl)
{
	m_nIsStatic = isStatic;
	//alloc buffer
	m_nElementCount = ntetelm;	
	m_pTetraElement = new CTetFem2Element[m_nElementCount];
	assert(m_pTetraElement!=0);

	//
	m_pSurfacePoly = new int [nface*3];
	assert(m_pSurfacePoly!=NULL);
	memcpy(m_pSurfacePoly, pFace, sizeof(int)*nface*3);
	m_nSurfacePoly = nface;

	//first, need to decide mass
	const double rho = m_mtl.getDensity();
	_computeNodeMass(pVertex, pElement, ntetelm, rho);

	//then, init each tetra2 element
	const Vector4i *pquad = (const Vector4i*)pElement;
	for (int i=0; i<m_nElementCount; i++){
		CTetFem2Element *p = &m_pTetraElement[i];
		const Vector4i quad = pquad[i];
		const Vector3d& p0 = pVertex[quad.x];
		const Vector3d& p1 = pVertex[quad.y];
		const Vector3d& p2 = pVertex[quad.z];
		const Vector3d& p3 = pVertex[quad.w];
		p->init(mtl, &quad.x, p0, p1, p2, p3);
	}
	
	printf("There are %d shell elements generated.\n", m_nElementCount);
}


void CSimuTetra2::updateState(void)
{
	for (int i=0; i<m_nElementCount; i++){
		m_pTetraElement[i].updateState();
	}
}


void CSimuTetra2::_computeElasticForces(const unsigned int timeid, const bool needjacobian)
{
#define TETELM_TYPET CTetFem2Element
	int i;
	Vector3d force[4];
	double3x3 jacobian[4];
	double3x3 *ppjac[4]={&jacobian[0], &jacobian[1], &jacobian[2], &jacobian[3]};
	if (!needjacobian)
		ppjac[0] = ppjac[1] = ppjac[2] = ppjac[3] = NULL;
	
	//compute elastic force
	VertexInfo *pv = m_pVertInfo;
	for (i=0; i<m_nElementCount; i++){
		TETELM_TYPET &E = m_pTetraElement[i];
		const int* pNodeBuffer = E.getElementNodeBuffer();
		const int ia = pNodeBuffer[0];
		const int ib = pNodeBuffer[1];
		const int ic = pNodeBuffer[2];
		const int id = pNodeBuffer[3];
		E.computeForce(pv[ia].m_pos, pv[ib].m_pos, pv[ic].m_pos, pv[id].m_pos, m_mtl, force, ppjac);
		pv[ia].m_force += force[0]; pv[ib].m_force += force[1];
		pv[ic].m_force += force[2]; pv[id].m_force += force[3];
		if (needjacobian){
			saveVertSitffMatrixIntoSparseMatrix(ia, ia, *(ppjac[0]));
			saveVertSitffMatrixIntoSparseMatrix(ib, ib, *(ppjac[1]));
			saveVertSitffMatrixIntoSparseMatrix(ic, ic, *(ppjac[2]));
			saveVertSitffMatrixIntoSparseMatrix(id, id, *(ppjac[3]));
		}	
	}
#undef TETELM_TYPET
}

//compute the acceleration vector for the current location 
void CSimuTetra2::computeAcceleration(const unsigned int timeid, const bool needJacobian)
{
	//init forces as zero and add gravity load
	initForceAndApplyGravity();

	//add elastic forces
	_computeElasticForces(timeid, needJacobian);

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
		const int *pnodeid = m_pTetraElement[i].getElementNodeBuffer();
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


bool CSimuTetra2::exportMeshPlt(FILE *fp)
{
	const int nSurfaceType = 3;
	return ExportMeshPLTSurface(m_pVertInfo, m_nVertexCount, 
		m_pSurfacePoly, nSurfaceType, m_nSurfacePoly, fp);
}
*/