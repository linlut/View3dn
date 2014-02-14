//FILE: simu_springobj.cpp
#include <geomath.h>
#include "simu_shell.h"
#include "obj_spring.h"

const double SIXTH = 1.0/6.0;

inline double VOLUME_OF_TET(const Vector3d& p2, const Vector3d& p3, const Vector3d& p4)
{
	const Vector3d p3Xp4 = CrossProd(p3, p4);
	const double vol = DotProd(p2, p3Xp4)*SIXTH;
	return vol;
}

CSimuSpringObj::~CSimuSpringObj(void)
{
	SafeDeleteArray(m_pSpringElement);
	SafeDeleteArray(m_pTet);
	SafeDeleteArray(m_pTetVolume);
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

//Mass computation on edges by the method of VanGader
static void CreateEdgesForTetsVanGader(const Vector3d *pVertex, 
	const int *_pelm, const int nelm, const double rho, CEdgeMap& edgemap)
{
	const Vector4i *pelm = (const Vector4i*)_pelm;
	const double K = 12.0/sqrt(2.0);
	for (int i=0; i<nelm; i++){
		const Vector4i tet = pelm[i];
		const Vector3d& p0 = pVertex[tet.x];
		const Vector3d& p1 = pVertex[tet.y];
		const Vector3d& p2 = pVertex[tet.z];
		const Vector3d& p3 = pVertex[tet.w];
		const double vol = computeTetrahedronVolume(p0, p1, p2, p3);
		const double edge_vol = vol / 6.0;
		const double edge_mass = edge_vol * rho ;
		const double l0 = pow(vol*K, 0.333333333333333333333333);
		Vector2i edges[6]={
			Vector2i(tet.x, tet.y), Vector2i(tet.x, tet.z), Vector2i(tet.x, tet.w), 
			Vector2i(tet.y, tet.z), Vector2i(tet.y, tet.w), Vector2i(tet.z, tet.w)
		};
		//for each of the 6 edges loop
		for (int j=0; j<6; j++){
			const Vector2i e = edges[j];
			const INT64KEY key = CSpringAuxStruct::INT2KEY(e.x, e.y);
			CSpringAuxStruct &s = edgemap[key];
			s.m_mass += edge_mass; 
			const double edgelen = Distance(pVertex[e.x], pVertex[e.y]);
			s.m_crossarea += edge_vol/edgelen;
			s.m_stiff += vol/(edgelen*edgelen);
			s.m_vertID[0] = e.x; s.m_vertID[1] = e.y; 
		}
	}
}

//Mass computation on edges by the method of Lloyd TVCG 2008 paper
static void CreateEdgesForTetsLloyd(
	const Vector3d *pVertex, 
	const int *_pelm, 
	const int nelm, 
	const double rho, 
	CEdgeMap& edgemap)
{
	const Vector4i *pelm = (const Vector4i*)_pelm;
	const double K = 12.0/sqrt(2.0);
	const double K2 = 2.0*sqrt(2.0)/25.0;
	for (int i=0; i<nelm; i++){
		const Vector4i tet = pelm[i];
		const Vector3d& p0 = pVertex[tet.x];
		const Vector3d& p1 = pVertex[tet.y];
		const Vector3d& p2 = pVertex[tet.z];
		const Vector3d& p3 = pVertex[tet.w];
		const double elm_vol = computeTetrahedronVolume(p0, p1, p2, p3);
		const double edge_vol = elm_vol / 6.0;
		const double edge_mass = edge_vol*rho;
		//effective tet length (as an equal-length tet)
		const double l0 = pow(elm_vol*K, 0.3333333333333333333333333);
		const double edgecross_area = elm_vol / (6.0 * l0);
		const double kij = K2*l0;
		const Vector2i edges[6]={
			Vector2i(tet.x, tet.y), Vector2i(tet.x, tet.z), Vector2i(tet.x, tet.w), 
			Vector2i(tet.y, tet.z), Vector2i(tet.y, tet.w), 
			Vector2i(tet.z, tet.w)
		};
		//for each of the 6 edges loop
		for (int j=0; j<6; j++){
			const Vector2i e = edges[j];
			const double edgelen = Distance(pVertex[e.x], pVertex[e.y]);
			const double crossArea = edge_vol/edgelen;
			const INT64KEY key = CSpringAuxStruct::INT2KEY(e.x, e.y);
			CSpringAuxStruct &s = edgemap[key];
			s.m_mass += edge_mass;				//mass on the edge 
			//s.m_crossarea += edgecross_area;	//for shear element method
			s.m_crossarea += crossArea;
			s.m_stiff += kij;					//for mass-spring method
			s.m_vertID[0] = e.x; s.m_vertID[1] = e.y; 
		}
	}
}

//Mass computation on edges of hexahedra
static void CreateEdgesForHexahedra(const int springtype, const Vector3d *pVertex, 
	const int *_pelm, const int nelm, const double rho, CEdgeMap& edgemap)
{
	//compute the edge link patten
	const Vector2i edgetab[16]={
		Vector2i(0, 1), Vector2i(1, 2),
		Vector2i(2, 3), Vector2i(3, 0), 
		Vector2i(4, 5), Vector2i(5, 6),
		Vector2i(6, 7), Vector2i(7, 4),
		Vector2i(0, 4), Vector2i(1, 5),
		Vector2i(2, 6), Vector2i(3, 7),
		Vector2i(0, 6), Vector2i(1, 7),
		Vector2i(2, 4), Vector2i(3, 5)
	};
	CEdgeMap tt;

	switch(springtype){
	default:
	case 0: //spring struct with 12 cube edges, 6 face diagonal, 4 cube diagonal
	case 3:
	case 6:
		for (int i=0; i<7; ++i){
			for (int j=i+1; j<8; ++j){
				const INT64KEY key = CSpringAuxStruct::INT2KEY(i, j);
				CSpringAuxStruct &s = tt[key];	
				s.m_vertID[0] = i; 
				s.m_vertID[1] = j;
			}
		}
		break;
	case 1:	//spring struct with 16 edges (12 + 4 diagonal)
	case 4:
	case 7:
	case 2: //spring struct with 12 edges only
	case 5:
	case 8:
		{
		int ne=12;
		if (springtype==1 || springtype==4 || springtype==7) 
			ne=16;
		for (int i=0; i<ne; ++i){
			const int ix = edgetab[i].x, iy =edgetab[i].y;
			const INT64KEY key = CSpringAuxStruct::INT2KEY(ix, iy);
			CSpringAuxStruct &s = tt[key];	
			s.m_vertID[0] = ix; 
			s.m_vertID[1] = iy;
		}
		}
		break;
	};

	//create a weight arrary for cube edges
	const int ncubeedge = tt.size();
	double weights[80];
	assert(ncubeedge<80);
	for (int i=0; i<ncubeedge; i++) weights[i]=1.0/ncubeedge;

	//Build all the tursses for all cubes
	const Vector8i *pelm = (const Vector8i*)_pelm;
	for (int i=0; i<nelm; ++i){
		const Vector8i& hex = pelm[i];
		const double elm_vol = computeHexahedronVolume(pVertex, hex);
		//compute edge mass weights, assue equal edge cross area
		if (springtype>=3){			
			double totalEdgeLen = 0;
			int edgeID = 0;
			for (CEdgeMap::iterator itr = tt.begin(); itr!=tt.end(); ++itr, ++edgeID){
				const CSpringAuxStruct &e = itr->second;
				const int v0 = hex[e.m_vertID[0]];
				const int v1 = hex[e.m_vertID[1]];
				double edgelength = Distance(pVertex[v0], pVertex[v1]);
				//weights with edge length or edge length^2
				if (springtype>=6){
					edgelength *= sqrt(edgelength) * (edgelength);  //^2.5
				}
				weights[edgeID] = edgelength;
				totalEdgeLen += edgelength;
			}
			for (int j=0; j<ncubeedge; ++j) weights[j]/=totalEdgeLen;
		}
		//init each edge
		int edgeID = 0;
		for (CEdgeMap::iterator itr=tt.begin(); itr!=tt.end(); ++itr, ++edgeID){
			const double edge_vol = elm_vol*weights[edgeID];
			const double edge_mass = edge_vol*rho;
			CSpringAuxStruct &e = itr->second;
			const int v0 = hex[e.m_vertID[0]];
			const int v1 = hex[e.m_vertID[1]];
			const INT64KEY key = CSpringAuxStruct::INT2KEY(v0, v1);
			CSpringAuxStruct &s = edgemap[key];
			s.m_mass += edge_mass; 
			const double edgelen = Distance(pVertex[v0], pVertex[v1]);
			s.m_crossarea += edge_vol/edgelen;
			s.m_stiff += edge_vol/(edgelen*edgelen);
			s.m_vertID[0] = v0; 
			s.m_vertID[1] = v1; 
		}
	}
}

void buildEdgeMapFromSolidElements(
	const int springtype,	//0: van Gedar 94?, 1: Lloyd 2008 TVCG
	const Vector3d *pVertex, const int elmtype, const int *pElement, const int nelm, const double rho, 
	CEdgeMap& edgemap)
{
	char hexobj[]="Hexobj";
	char tetobj[]="Tetobj";
	const char *oname=NULL;
	assert(elmtype==4 || elmtype==8);
	if (elmtype==4){//Tet mesh
		oname = tetobj;
		switch (springtype){
			case 0:	//Van Gader's method
				CreateEdgesForTetsVanGader(pVertex, pElement, nelm, rho, edgemap);
				break;
			case 1: //Lloyd's method TVCG2007
				CreateEdgesForTetsLloyd(pVertex, pElement, nelm, rho, edgemap);
				break;
			default:
				CreateEdgesForTetsLloyd(pVertex, pElement, nelm, rho, edgemap);
				break;
		};
	}
	else{
		//elmtype==8
		assert(elmtype==8);
		oname = hexobj;
		CreateEdgesForHexahedra(springtype, pVertex, pElement, nelm, rho, edgemap);
	}
	printf("%s edge map size is %d\n", oname, edgemap.size());
}


CSimuSpringObj::CSimuSpringObj(
	const int springtype,
	const Vector3d *pVertex, const int nv,		//vertex array and length	
	const Vector3d &init_velocity,				//initial vertex velocity
	const int isSolid,							//solid or surface 1: solid, 0: surface
	const int *pElement, const int nv_per_elm, const int nelm,	//element array
	const double thickness,
	const CMeMaterialProperty &_mtl)
	:CSimuEntity(pVertex, nv, init_velocity, &_mtl)
{
	m_pSurfacePoly=NULL;
	m_nSurfaceType=0;		//=3 or 4, number of nodes per surface element
	m_nSurfacePoly=0;		//# of surface polygon
	m_nElement = nelm;
	m_nElementType = nv_per_elm;
	m_pTet = NULL;
	m_pTetVolume = NULL;
	assert(m_nElementType==3 || m_nElementType==4 || m_nElementType==8);

	//first, need to decide mass
	m_mtl = _mtl;
	const double rho = m_mtl.getDensity();
	_computeNodeMass(pVertex, isSolid, pElement, nv_per_elm, nelm, thickness, rho);

	//create strings using different tech to assign the stiffness ratio
	CEdgeMap edgemap;
	if (isSolid){
		buildEdgeMapFromSolidElements(springtype, pVertex, m_nElementType, pElement, nelm, rho, edgemap);
	}
	else{
		NOT_IMPLEMENTED();
	}

	//allocate the buffer and init
	m_nElementCount = edgemap.size();
	m_pSpringElement = new CSpringElement[m_nElementCount];
	assert(m_pSpringElement!=NULL);
	CEdgeMap::iterator itr=edgemap.begin();
	int scount=0;
	const double youngs = m_mtl.getYoung();
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

	//store elements for volume preservation forces
	if (isSolid && (m_nElementType==4)){
		int i;
		const int elmbufsize = nv_per_elm*nelm;
		m_pTet = new int [elmbufsize];
		assert(m_pTet!=NULL);
		for (i=0; i<elmbufsize; i++) m_pTet[i] = pElement[i];
		//init volume
		m_pTetVolume = new double [nelm];
		assert(m_pTetVolume!=NULL);
		const Vector4i *pelm = (Vector4i*)m_pTet;
		for (i=0; i<nelm; i++){
			const Vector4i e = pelm[i];
			const Vector3d &p0 = m_pVertInfo[e.x].m_pos;
			const Vector3d &p1 = m_pVertInfo[e.y].m_pos;
			const Vector3d &p2 = m_pVertInfo[e.z].m_pos;
			const Vector3d &p3 = m_pVertInfo[e.w].m_pos;
			m_pTetVolume[i] = VOLUME_OF_TET(p1-p0, p2-p0, p3-p0);
		}
	}
}

void CSimuSpringObj::setBoundarySurface(
	const int* pSurfacePoly,					//surface polygon array
	const int nSurfaceType,						//=3 or 4, number of nodes per surface element
	const int nSurfacePoly)						//# of surface polygon
{
	SafeDeleteArray(m_pSurfacePoly);
	const int nsize = nSurfaceType * nSurfacePoly;
	m_pSurfacePoly = new int [nsize];
	assert(m_pSurfacePoly!=NULL);
	for (int i=0; i<nsize; i++) m_pSurfacePoly[i]=pSurfacePoly[i];
	m_nSurfaceType = nSurfaceType;
	m_nSurfacePoly = nSurfacePoly;
}

inline void ComputeTetVolumePreservationForce(
	const double K,			//stiffness
	const double V0,		//initial volume
	const Vector3d &_p1, const Vector3d &_p2, const Vector3d &_p3, const Vector3d &_p4,	
	Vector3d &f1, Vector3d &f2, Vector3d &f3, Vector3d &f4)		
{
	const Vector3d p2 = _p2 - _p1;
	const Vector3d p3 = _p3 - _p1;
	const Vector3d p4 = _p4 - _p1;

	const Vector3d p3Xp4 = CrossProd(p3, p4);
	const double vol = DotProd(p2, p3Xp4)*SIXTH;
	const double k = -K*(vol - V0)*SIXTH;
	f4 = CrossProd(p2, p3)*k;
	f3 = CrossProd(p4, p2)*k;
	f2 = p3Xp4*k;
	f1 = -(f2+f3+f4);
}

void CSimuSpringObj::_computeElasticForces(const bool needjacobian)
{
	int i;
	Vector3d force;
	double3x3 jacobian, *pjac=NULL;
	if (needjacobian) pjac = &jacobian;
	//spring forces
	const double invdt = 1.0/m_mtl.getTimeStep();
	const double Kvd = m_mtl.getVelocityDamping()*invdt;
	for (i=0; i<m_nElementCount; i++){
		CSpringElement &E = m_pSpringElement[i];
		const int* pNodeBuffer = E.getElementNodeBuffer();
		const int ia = pNodeBuffer[0];
		const int ib = pNodeBuffer[1];
		Vector3d& p0 = m_pVertInfo[ia].m_pos;
		Vector3d& p1 = m_pVertInfo[ib].m_pos;
		Vector3d& oldp0 = m_pVertInfo[ia].m_pos0;
		Vector3d& oldp1 = m_pVertInfo[ib].m_pos0;
		const Vector3d v0 = (p0 - oldp0)*invdt;
		const Vector3d v1 = (p1 - oldp1)*invdt;
		E.computeNodalForce(p0, p1, v0, v1, Kvd, force, pjac);
		m_pVertInfo[ia].m_force += force;
		m_pVertInfo[ib].m_force -= force;
		if (needjacobian){
			saveVertSitffMatrixIntoSparseMatrix(ia, ib, jacobian);
		}
	}
	//volume preservation forces
	return;
	if (m_pTet!=NULL){
		const Vector4i *pelm = (Vector4i*)m_pTet;
		const double K = m_mtl.getYoung()*m_mtl.getYoung()*1000*0.8;
		Vector3d f0, f1, f2, f3;
		for (i=0; i<m_nElement; i++){
			const Vector4i e = pelm[i];
			const Vector3d &p0 = m_pVertInfo[e.x].m_pos;
			const Vector3d &p1 = m_pVertInfo[e.y].m_pos;
			const Vector3d &p2 = m_pVertInfo[e.z].m_pos;
			const Vector3d &p3 = m_pVertInfo[e.w].m_pos;
			const double V0 = m_pTetVolume[i];
			ComputeTetVolumePreservationForce(K, V0, p0, p1, p2, p3, f0, f1, f2, f3);
			m_pVertInfo[e.x].m_force+=f0;
			m_pVertInfo[e.y].m_force+=f1;
			m_pVertInfo[e.z].m_force+=f2;
			m_pVertInfo[e.w].m_force+=f3;
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
bool CSimuSpringObj::_exportMeshPLTLine(FILE *fp)
{
	char PLYTYPESTR[16]="LINE";
	//vertex array
	fprintf(fp, "TITLE = SHELLSIMULATION\n");
	fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\"\n");
	fprintf(fp, "ZONE N = %d  E = %d F = FEPOINT, ET = %s\n", m_nVertexCount, m_nElementCount, PLYTYPESTR);
	for (int i=0; i<m_nVertexCount; i++){
        const Vector3d p= m_pVertInfo[i].m_pos;
		fprintf(fp, "%.12lG %.12lG %.12lG\n", p.x, p.y, p.z);
    }
	//mesh connectivity
    for (int i=0; i<m_nElementCount; i++){
		const int *pnodeid = m_pSpringElement[i].getElementNodeBuffer();
		int x = pnodeid[0];
		int y = pnodeid[1];
		x++; y++;
		fprintf(fp, "%d %d\n", x, y);
	}
	return 1;
}

bool CSimuSpringObj::_exportMeshPLTSurface(FILE *fp)
{
	return ExportMeshPLTSurface(
				m_pVertInfo, m_nVertexCount, 
				m_pSurfacePoly, m_nSurfaceType, m_nSurfacePoly, 
				fp);
}

void CSimuSpringObj::exportMesh(const int count, const char *fname, const char *ext)
{
	char ffname[200];
	sprintf(ffname, "spring%d.plt", count);

	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return;
	exportMeshPlt(fp);
	if (fp) fclose(fp);
}


bool CSimuSpringObj::exportMeshPlt(FILE *fp)
{
	bool r;
	if (m_pSurfacePoly==NULL)
		r = _exportMeshPLTLine(fp);
	else
		r = _exportMeshPLTSurface(fp);
	return r;
}
