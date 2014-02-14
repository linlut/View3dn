//FILE: simu_springobj.cpp

#include <map>
#include <geomath.h>
#include <view3dn/edgetable.h>
#include "simu_shell.h"
#include "simu_springobj.h"

typedef CEdgeAuxStruct CSpringAuxStruct;
typedef map<INT64KEY, CSpringAuxStruct> CEdgeMap;


CSimuSpringObj::~CSimuSpringObj(void)
{
	SafeDeleteArray(m_pSpringElement);
}


void CSimuSpringObj::_computeNodeMass(
	const Vector3d *pVertex, 
	const int isSolid,
	const int *pElement, const int nv_per_elm, const int nelm, 
	const double &thickness, const double &rho)
{
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	if (!isSolid){
		//for surface mesh computation
		ComputeNodeMassForSurface(pVertex, m_nVertexCount, pElement, nv_per_elm, nelm, thickness, rho, m_pVertInfo);
		return;
	}

	if (nv_per_elm==4){
		//Tetrahedral mesh
		ComputeNodeMassTet(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
	}
	else{
		//Hexahedral mesh
		ComputeNodeMassHex(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
	}
}

static void CreateEdgesForTetsVanGader(const Vector3d *pVertex, 
	const int *_pelm, const int nelm, const double rho, CEdgeMap& edgemap)
{
	const Vector4i *pelm = (const Vector4i*)_pelm;
	for (int i=0; i<nelm; i++){
		const Vector4i tet = pelm[i];
		const Vector3d& p0 = pVertex[tet.x];
		const Vector3d& p1 = pVertex[tet.y];
		const Vector3d& p2 = pVertex[tet.z];
		const Vector3d& p3 = pVertex[tet.w];
		const double vol = computeTetrahedronVolume(p0, p1, p2, p3);
		const double mass = vol*rho*0.25;
		Vector2i edges[6]={
			Vector2i(tet.x, tet.y), Vector2i(tet.x, tet.z), Vector2i(tet.x, tet.w), 
			Vector2i(tet.y, tet.z), Vector2i(tet.y, tet.w), Vector2i(tet.z, tet.w)
		};
		//for each of the 6 edges loop
		for (int j=0; j<6; j++){
			Vector2i e = edges[j];
			INT64KEY key = CSpringAuxStruct::INT2KEY(e.x, e.y);
			CSpringAuxStruct &s = edgemap[key];
			s.m_mass += mass; 
			s.m_stiff += vol/Distance2(pVertex[e.x], pVertex[e.y]);
			s.m_vertID[0] = e.x; s.m_vertID[1] = e.y; 
		}
	}
}

static void CreateEdgesForTetsLloyd(const Vector3d *pVertex, 
	const int *_pelm, const int nelm, const double rho, CEdgeMap& edgemap)
{
	const Vector4i *pelm = (const Vector4i*)_pelm;
	const double K = 12.0/sqrt(2.0);
	const double K2 = 2.0*sqrt(2.0)/25;
	for (int i=0; i<nelm; i++){
		const Vector4i tet = pelm[i];
		const Vector3d& p0 = pVertex[tet.x];
		const Vector3d& p1 = pVertex[tet.y];
		const Vector3d& p2 = pVertex[tet.z];
		const Vector3d& p3 = pVertex[tet.w];
		const double vol = computeTetrahedronVolume(p0, p1, p2, p3);
		const double mass = vol*rho*0.25;
		//effective tet length (as an equal-length tet)
		const double l0 = pow(vol*K, 0.3333333333333333);
		const double kij = K2*l0;
		Vector2i edges[6]={
			Vector2i(tet.x, tet.y), Vector2i(tet.x, tet.z), Vector2i(tet.x, tet.w), 
			Vector2i(tet.y, tet.z), Vector2i(tet.y, tet.w), Vector2i(tet.z, tet.w)
		};
		//for each of the 6 edges loop
		for (int j=0; j<6; j++){
			Vector2i e = edges[j];
			INT64KEY key = CSpringAuxStruct::INT2KEY(e.x, e.y);
			CSpringAuxStruct &s = edgemap[key];
			s.m_mass += mass; 
			s.m_stiff += kij;
			s.m_vertID[0] = e.x; s.m_vertID[1] = e.y; 
		}
	}
}

CSimuSpringObj::CSimuSpringObj(
	const int springtype,
	const Vector3d *pVertex, const int nv,		//vertex array and length	
	const Vector3d &init_velocity,				//initial vertex velocity
	const int isSolid,							//solid or surface 1: solid, 0: surface
	const int *pElement, const int nv_per_elm, const int nelm,	//element array
	const double thickness,
	const CMeMaterialProperty &mtl)
	:CSimuEntity(pVertex, nv, init_velocity)
{
	//a lot of things to do
	const int m_elmtype = nv_per_elm;
	assert(m_elmtype==3 || m_elmtype==4 || m_elmtype==8);

	//first, need to decide mass
	_computeNodeMass(pVertex, isSolid, pElement, nv_per_elm, nelm, thickness, mtl.m_rho);

	//create strings using different tech to assign the stiffness ratio
	CEdgeMap edgemap;
	if (isSolid){
		switch (springtype){
			case 0:	//Van Gader's method
				CreateEdgesForTetsVanGader(pVertex, pElement, nelm, mtl.m_rho, edgemap);
				break;
			case 1: //Lloyd's method TVCG2007
				CreateEdgesForTetsLloyd(pVertex, pElement, nelm, mtl.m_rho, edgemap);
				break;
			default:
				CreateEdgesForTetsLloyd(pVertex, pElement, nelm, mtl.m_rho, edgemap);
				break;
		};
	}
	else{
		NOT_IMPLEMENTED();
	}

	//allocate the buffer and init
	m_nElementCount = edgemap.size();
	m_pSpringElement = new CSpringElement[m_nElementCount];
	assert(m_pSpringElement!=NULL);
	map<INT64KEY, CSpringAuxStruct>::iterator itr=edgemap.begin();
	int scount=0;
	const double youngs = mtl.getYoung();
	while (itr!=edgemap.end()){
		CSpringAuxStruct &e = itr->second;
		CSpringElement &s = m_pSpringElement[scount];
		const int *vid = e.m_vertID;
		const double K = e.m_stiff*youngs;
		const double KD = 0;
		s.init(vid, pVertex[vid[0]], pVertex[vid[1]], K, KD);
		itr++;
		scount++;
	}
	assert(scount == m_nElementCount);
}


void CSimuSpringObj::_computeElasticForces(const bool needjacobian)
{
	Vector3d force;
	double3x3 jacobian, *pjac=NULL;
	if (needjacobian) pjac = &jacobian;

	for (int i=0; i<m_nElementCount; i++){
		CSpringElement &E = m_pSpringElement[i];
		const int* pNodeBuffer = E.getElementNodeBuffer();
		const int ia = pNodeBuffer[0];
		const int ib = pNodeBuffer[1];
		Vector3d& p0 = m_pVertInfo[ia].m_pos;
		Vector3d& p1 = m_pVertInfo[ib].m_pos;
		E.computeNodalForce(p0, p1, force, pjac);
		m_pVertInfo[ia].m_force += force;
		m_pVertInfo[ib].m_force -= force;
		if (needjacobian){
			saveVertSitffMatrixIntoSparseMatrix(ia, ib, jacobian);
		}
	}
}

//compute the acceleration vector for the current location 
void CSimuSpringObj::computeAcceleration(const unsigned int timeid, const bool needJacobian)
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

//export mesh
bool CSimuSpringObj::_exportMeshPLT(const char *fname)
{
	int i;
	char PLYTYPESTR[16]="LINE";
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return false;
	//vertex array
	fprintf(fp, "TITLE = SHELLSIMULATION\n");
	fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\"\n");
	fprintf(fp, "ZONE N = %d  E = %d F = FEPOINT, ET = %s\n", m_nVertexCount, m_nElementCount, PLYTYPESTR);
	for (i=0; i<m_nVertexCount; i++){
        const Vector3d p= m_pVertInfo[i].m_pos;
		fprintf(fp, "%.12lG %.12lG %.12lG\n", p.x, p.y, p.z);
    }
	//mesh connectivity
    for (i=0; i<m_nElementCount; i++){
		const int *pnodeid = m_pSpringElement[i].getElementNodeBuffer();
		int x = pnodeid[0];
		int y = pnodeid[1];
		x++; y++;
		fprintf(fp, "%d %d\n", x, y);
	}
	fclose(fp);
	return 1;
}


void CSimuSpringObj::exportMesh(const int count, const char *fname, const char *ext)
{
	char ffname[200];
	sprintf(ffname, "spring%d.plt", count);

	_exportMeshPLT(ffname);
}
