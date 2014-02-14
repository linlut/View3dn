//FILE: simu_thinshell.cpp
#include <map>
#include <geomath.h>
#include <view3dn/edgetable.h>
#include "simu_shell.h"


void ComputeNodeMassForSurface(const Vector3d *pVertex, const int nv,
	const int *pElement, const int nv_per_elm, const int nelm, 
	const double &thickness, const double &rho, 
	CSimuEntity::VertexInfo *m_pVertInfo)
{
	int i;
	if (nv_per_elm==3){
		//First, init node mass as zero
		for (i=0; i<nv; i++) m_pVertInfo[i].m_mass = 0;
		//loop the elements;
		const Vector3i *ptri = (const Vector3i *)pElement;
		const double K = thickness * rho /3.0;
		for (i=0; i<nelm; i++){
			const Vector3i tri = ptri[i];
			double area = triangle_area(pVertex[tri.x], pVertex[tri.y], pVertex[tri.z]);
			const double mass = area * K;
			m_pVertInfo[tri.x].m_mass += mass;
			m_pVertInfo[tri.y].m_mass += mass;
			m_pVertInfo[tri.z].m_mass += mass;
		}
	}
	else{
		assert(nv_per_elm==4);
		//First, init node mass as zero
		for (i=0; i<nv; i++) m_pVertInfo[i].m_mass = 0;
		//loop the elements;
		const Vector4i *pquad = (const Vector4i *)pElement;
		const double K = thickness * rho /4.0;
		for (i=0; i<nelm; i++){
			const Vector4i quad = pquad[i];
			double area = triangle_area(pVertex[quad.x], pVertex[quad.y], pVertex[quad.z]);
			area += triangle_area(pVertex[quad.x], pVertex[quad.z], pVertex[quad.w]);
			const double mass = area * K;
			m_pVertInfo[quad.x].m_mass += mass;
			m_pVertInfo[quad.y].m_mass += mass;
			m_pVertInfo[quad.z].m_mass += mass;
			m_pVertInfo[quad.w].m_mass += mass;
		}
	}
}

//============================================================================

CSimuShell::CSimuShell(
		const Vector3d *pVertex, const int nv,		//vertex array and length	
		const Vector3d &init_velocity,				//initial vertex velocity
		const int *pElement, const int nv_per_elm, const int nelm,	//surf. elm array, not shell elm
		const double &shell_thickness,				//shell thinkness, seems not useful
		const CMeMaterialProperty& mtl,				//material property
		const int isStatic							//static or dynamic simulation
		):
	CSimuEntity(pVertex, nv, init_velocity, &mtl)
{
	//a lot of things to do
	m_thickness = shell_thickness;
	m_nSurfaceType = nv_per_elm;
	assert(m_nSurfaceType==3 || m_nSurfaceType==4);
	m_nSurfacePoly = nelm;
	m_static = isStatic;

	m_pSurfacePoly = new int [nv_per_elm*nelm];
	assert(m_pSurfacePoly!=NULL);
	for (int i=0; i<nv_per_elm*nelm; i++) m_pSurfacePoly[i]=pElement[i];
	m_pSurfaceNorm = new Vector3d[nelm];
	assert(m_pSurfaceNorm!=NULL);

	//prepare for relaxation
	m_pRelaxationEdge=NULL;		
	m_nRelaxationEdge= 0;		
	if (mtl.m_rigid) buildRelaxationEdges(pVertex, mtl.m_rigid);
}


void CSimuShell::computeNodeMass(const Vector3d *pVertex, const double &rho)
{
	ASSERT0(m_pVertInfo!=NULL);
	ASSERT0(m_nVertexCount>0);
	ASSERT0(m_pSurfacePoly!=NULL);
	ComputeNodeMassForSurface(pVertex, m_nVertexCount, 
		m_pSurfacePoly, m_nSurfaceType, m_nSurfacePoly, m_thickness, rho, m_pVertInfo);
}


static inline Vector3d _getVertexUsingStride(const Vector3d *parray, const int n, const int stride)
{
	const char *p = (const char *)parray;
	p += stride*n;
	return *((const Vector3d *)p);
}


void CSimuShell::computeSurfaceNormal(const Vector3d *pVertex, const int stride)
{
	assert(m_pSurfaceNorm!=NULL);
	if (m_nSurfaceType==3){
		const Vector3i *ptri = (const Vector3i *)m_pSurfacePoly;
		for (int i=0; i<m_nSurfacePoly; i++){
			const Vector3i& tri = ptri[i];
			const Vector3d a = _getVertexUsingStride(pVertex, tri.x, stride);
			const Vector3d b = _getVertexUsingStride(pVertex, tri.y, stride);
			const Vector3d c = _getVertexUsingStride(pVertex, tri.z, stride);
			m_pSurfaceNorm[i] = compute_triangle_normal(a, b, c);
		}
	}
	else{
		ASSERT0(m_nSurfaceType==4);
		const Vector4i *ptri = (const Vector4i *)m_pSurfacePoly;
		for (int i=0; i<m_nSurfacePoly; i++){
			const Vector4i& tri = ptri[i];
			const Vector3d a = _getVertexUsingStride(pVertex, tri.x, stride);
			const Vector3d b = _getVertexUsingStride(pVertex, tri.y, stride);
			const Vector3d c = _getVertexUsingStride(pVertex, tri.z, stride);
			const Vector3d d = _getVertexUsingStride(pVertex, tri.w, stride);
			m_pSurfaceNorm[i] = compute_quad_normal(a, b, c, d);
		}
	}
}


void CSimuShell::computeSurfaceAreaWeightedNormal(const Vector3d *pVertex, const int stride)
{
	assert(m_pSurfaceNorm!=NULL);
	if (m_nSurfaceType==3){
		const Vector3i *ptri = (const Vector3i *)m_pSurfacePoly;
		for (int i=0; i<m_nSurfacePoly; i++){
			const Vector3i& tri = ptri[i];
			const Vector3d a = _getVertexUsingStride(pVertex, tri.x, stride);
			const Vector3d b = _getVertexUsingStride(pVertex, tri.y, stride);
			const Vector3d c = _getVertexUsingStride(pVertex, tri.z, stride);
			compute_triangle_areanormal(a, b, c, m_pSurfaceNorm[i]);
		}
	}
	else{
		ASSERT0(m_nSurfaceType==4);
		const Vector4i *ptri = (const Vector4i *)m_pSurfacePoly;
		for (int i=0; i<m_nSurfacePoly; i++){
			const Vector4i& tri = ptri[i];
			const Vector3d a = _getVertexUsingStride(pVertex, tri.x, stride);
			const Vector3d b = _getVertexUsingStride(pVertex, tri.y, stride);
			const Vector3d c = _getVertexUsingStride(pVertex, tri.z, stride);
			const Vector3d d = _getVertexUsingStride(pVertex, tri.w, stride);
			compute_quad_areanormal(a, b, c, d, m_pSurfaceNorm[i]);
		}
	}
}


//export mesh
bool CSimuShell::_exportMeshPLT(FILE *fp)
{
	int i;
	char PLYTYPESTR[2][16]={"TRIANGLE", "QUADRILATERAL"};
	const int stridx = m_nSurfaceType - 3;
	//vertex array
	const int nv = m_nVertexCount;
	const int npoly = m_nSurfacePoly;
	fprintf(fp, "TITLE = SHELLSIMULATION\n");
	fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\"\n");
	fprintf(fp, "ZONE N = %d  E = %d F = FEPOINT, ET = %s\n", nv, npoly, PLYTYPESTR[stridx]);
	for (i=0; i<nv; i++){
        const Vector3d p= m_pVertInfo[i].m_pos;
		fprintf(fp, "%.12lG %.12lG %.12lG\n", p.x, p.y, p.z);
    }
	//mesh connectivity
    for (i=0; i<npoly; i++){
		const int *pnodeid = &m_pSurfacePoly[i*m_nSurfaceType];
		int x = pnodeid[0];
		int y = pnodeid[1];
		int z = pnodeid[2];
		int w = pnodeid[3];
		x++; y++; z++; w++;
		if (m_nSurfaceType==3){
			fprintf(fp, "%d %d %d\n", x, y, z);
		}
		else{
			fprintf(fp, "%d %d %d %d\n", x, y, z, w);
		}
	}
	return 1;
}

//export mesh
bool CSimuShell::_exportMeshTXT(const char *fname)
{
	int i;
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return false;
	//vertex array
	const int nv = m_nVertexCount;
	const int npoly = m_nSurfacePoly;
	fprintf(fp, "%d %d\n1\n", nv, npoly);
	for (i=0; i<nv; i++){
        const Vector3d p= m_pVertInfo[i].m_pos;
		fprintf(fp, "%.12lG %.12lG %.12lG\n", p.x, p.y, p.z);
    }
	//mesh connectivity
    for (i=0; i<npoly; i++){
		const int *pnodeid = &m_pSurfacePoly[i*m_nSurfaceType];;
		const int x = pnodeid[0];
		const int y = pnodeid[1];
		const int z = pnodeid[2];
		const int w = pnodeid[3];
		fprintf(fp, "%d %d %d\n", x, y, z);
		fprintf(fp, "%d %d %d\n", y, x, w);
	}
	fclose(fp);
	return 1;
}

void CSimuShell::exportMesh(const int count, const char *fname, const char *ext)
{
	char ffname[200];
	sprintf(ffname, "shell%d.plt", count);

	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return;
	_exportMeshPLT(fp);
	if (fp) fclose(fp);
}


bool CSimuShell::exportMeshPlt(FILE *fp)
{
	bool r = _exportMeshPLT(fp);
	return r;
}


typedef map<INT64KEY, CEdgeAuxStruct> CEdgeMap;
static void CreateEdgesForTriMesh(const Vector3i *pelm, const int nelm, CEdgeMap& edgemap)
{
	for (int i=0; i<nelm; i++){
		const Vector3i tri = pelm[i];
		Vector2i edges[3]={
			Vector2i(tri.x, tri.y), 
			Vector2i(tri.y, tri.z), 
			Vector2i(tri.z, tri.x)};
		for (int j=0; j<3; j++){
			const Vector2i& e = edges[j];
			INT64KEY key = CEdgeAuxStruct::INT2KEY(e.x, e.y);
			CEdgeAuxStruct &s = edgemap[key];
			s.m_vertID[0] = e.x; s.m_vertID[1] = e.y; 
		}
	}
}
static void CreateEdgesForQuadMesh(const int rigidlevel, const Vector4i *pelm, const int nelm, CEdgeMap& edgemap)
{
	if (rigidlevel<1) return;
	for (int i=0; i<nelm; i++){
		const Vector4i quad = pelm[i];
		Vector2i edges[4]={
			Vector2i(quad.x, quad.y), 
			Vector2i(quad.y, quad.z), 
			Vector2i(quad.z, quad.w),
			Vector2i(quad.w, quad.x)};
		for (int j=0; j<4; j++){
			const Vector2i& e = edges[j];
			INT64KEY key = CEdgeAuxStruct::INT2KEY(e.x, e.y);
			CEdgeAuxStruct &s = edgemap[key];
			s.m_vertID[0] = e.x; s.m_vertID[1] = e.y; 
		}
	}
	if (rigidlevel<3) return;
	for (int i=0; i<nelm; i++){
		const Vector4i quad = pelm[i];
		Vector2i edges[2]={Vector2i(quad.x, quad.z), Vector2i(quad.y, quad.w)};
		for (int j=0; j<2; j++){
			const Vector2i& e = edges[j];
			INT64KEY key = CEdgeAuxStruct::INT2KEY(e.x, e.y);
			CEdgeAuxStruct &s = edgemap[key];
			s.m_vertID[0] = e.x; s.m_vertID[1] = e.y; 
		}
	}
}

void CSimuShell::buildRelaxationEdges(const Vector3d *pVertex, const int rigidlevel)
{
	//parameter check
	if (m_pSurfacePoly==NULL || m_nSurfacePoly==0) return;
	assert(m_nSurfaceType==3 || m_nSurfaceType==4);
	if (rigidlevel==0) return;

	CEdgeMap edgemap;
	if (m_nSurfaceType==3)
		CreateEdgesForTriMesh((const Vector3i*)m_pSurfacePoly, m_nSurfacePoly, edgemap);
	else
		CreateEdgesForQuadMesh(rigidlevel, (const Vector4i*)m_pSurfacePoly, m_nSurfacePoly, edgemap);
	
	//copy the edges into an array
	m_nRelaxationEdge = edgemap.size();
	m_pRelaxationEdge = new CRelaxationEdge[m_nRelaxationEdge];
	assert(m_pRelaxationEdge!=NULL);
	CEdgeMap::iterator itr;
	int cc = 0;
	double avg_edgelen = 0;
	for (itr=edgemap.begin(); itr!=edgemap.end(); itr++, cc++){
		CEdgeAuxStruct &s = itr->second;
		const int x = s.m_vertID[0];
		const int y = s.m_vertID[1];
		m_pRelaxationEdge[cc].x = x;
		m_pRelaxationEdge[cc].y = y;
		const double len = Distance(pVertex[x], pVertex[y]);
		avg_edgelen += len;
		m_pRelaxationEdge[cc].restlength2 = len*len;
	}	

	//if rigid level=1, just the small length edges
	if (rigidlevel==1){
		const double LIMITRATIO=0.30;
		avg_edgelen/=m_nRelaxationEdge;
		const double smalledgelen = avg_edgelen *LIMITRATIO;
		const double smalledgelen2 = smalledgelen * smalledgelen;
		int i, j;
		for (i=j=0; i<m_nRelaxationEdge; i++){
			CRelaxationEdge *pedge = &m_pRelaxationEdge[i]; 
			if (pedge->restlength2<smalledgelen2){
				m_pRelaxationEdge[j] = *pedge;
				j++;
			}
		}
		printf("Total edges %d, small edges %d generated.\n", m_nRelaxationEdge, j);
		m_nRelaxationEdge = j;
		if (m_nRelaxationEdge==0){
			SafeDeleteArray(m_pRelaxationEdge);
			m_pRelaxationEdge = NULL;
		}
	}
}


inline void CSimuShell::_performEdgeRelaxationOneIterationUp(const double &DLEN_TOL, const double &KK)
{
	for (int i=0; i<m_nRelaxationEdge; i++){
		const CRelaxationEdge &e = m_pRelaxationEdge[i];
		const int ix = e.x;
		const int iy = e.y;
		const double l0l0 = e.restlength2;
		Vector3d& px = m_pVertInfo[ix].m_pos;
		Vector3d& py = m_pVertInfo[iy].m_pos;
		Vector3d delta = py - px;
		//to get rid of the sqrt operation, we use the following from GDC2001
		const double deltadelta = DotProd(delta, delta);
		delta*=l0l0/(deltadelta+l0l0) - 0.5;
		px -= delta;
		py += delta;
	}
}

inline void CSimuShell::_performEdgeRelaxationOneIterationDown(const double &DLEN_TOL, const double &KK)
{
	for (int i=m_nRelaxationEdge-1; i>=0; i--){
		const CRelaxationEdge &e = m_pRelaxationEdge[i];
		const int ix = e.x;
		const int iy = e.y;
		const double l0l0 = e.restlength2;
		Vector3d& px = m_pVertInfo[ix].m_pos;
		Vector3d& py = m_pVertInfo[iy].m_pos;
		Vector3d delta = py - px;
		const double deltadelta = DotProd(delta, delta);
		delta*= (l0l0/(deltadelta+l0l0) - 0.50);
		px -= delta;
		py += delta;
	}
}


void CSimuShell::performEdgeRelaxation(const double &current_tm, const double &dt)
{
	if (m_pRelaxationEdge==NULL || m_nRelaxationEdge==0){
		CSimuEntity::performEdgeRelaxation(current_tm, dt);		
		return;
	}
	const double DLEN_TOL = 0.010;
	const int loopnum = 2;
	const double KK = 0.400;
	for (int j=0; j<loopnum; j++){
		//if (current_tm<5000){
		if ((j&0x1)==0)
			_performEdgeRelaxationOneIterationUp(DLEN_TOL, KK);
		else
			_performEdgeRelaxationOneIterationDown(DLEN_TOL, KK);
		//}
		applyAllPositionConstraints(current_tm, dt);		
	}
}


int CSimuShell::_exportOBJFileObjectQuad(const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const
{
	assert(m_nSurfaceType==4);
	Vector4i *ptri = (Vector4i *)m_pSurfacePoly; 
	Vector4i vertexbase(vertexbaseid, vertexbaseid, vertexbaseid, vertexbaseid);
	Vector4i vertextexbase(texbaseid, texbaseid, texbaseid, texbaseid);
	for (int i=0; i<m_nSurfacePoly; i++){
		const Vector4i tri = ptri[i] + vertexbase;
		if (m_pTextureCoord==NULL){
			fprintf(fp, "f %d %d %d %d\n", tri.x, tri.y, tri.z, tri.w);
		}
		else{
			const Vector4i tex = ptri[i] + vertextexbase;
			fprintf(fp, "f %d/%d %d/%d %d/%d %d/%d\n", 
						tri.x, tex.x,
						tri.y, tex.y,
						tri.z, tex.z,
						tri.w, tex.w);
		}
	}
	return m_nVertexCount;
}

int CSimuShell::_exportOBJFileObjectTri(const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const
{
	assert(m_nSurfaceType==3);
	Vector3i *ptri = (Vector3i *)m_pSurfacePoly; 
	Vector3i vertexbase(vertexbaseid, vertexbaseid, vertexbaseid);
	Vector3i vertextexbase(texbaseid, texbaseid, texbaseid);
	for (int i=0; i<m_nSurfacePoly; i++){
		const Vector3i tri = ptri[i] + vertexbase;
		if (m_pTextureCoord==NULL){
			fprintf(fp, "f %d %d %d\n", tri.x, tri.y, tri.z);
		}
		else{
			const Vector3i tex = ptri[i] + vertextexbase;
			fprintf(fp, "f %d/%d %d/%d %d/%d\n", 
						tri.x, tex.x,
						tri.y, tex.y,
						tri.z, tex.z);
		}
	}
	return m_nVertexCount;
}

int CSimuShell::exportOBJFileObject(const int stepid, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const
{
	CSimuEntity::exportOBJFileObject(stepid, objid, vertexbaseid, texbaseid, fp);
	fprintf(fp, "usemtl mat%d\n", objid);
	if (m_nSurfaceType==3){
		_exportOBJFileObjectTri(objid, vertexbaseid, texbaseid, fp);
	}
	else{
		_exportOBJFileObjectQuad(objid, vertexbaseid, texbaseid, fp);
	}
	fprintf(fp, "\n", objid);
	return m_nVertexCount;
}
