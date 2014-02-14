//FILE: gyrod_lut.cpp
#include <ztime.h>
#include <math.h>
#include "gyrod_lut.h"
#include "element_shear.h"


int DGCubeMap::_getCloestAnchorVertex(const Vector3d &pos, const AnchorVertexInfo *pVertex, const int NU, const int NV) const
{
	const Vector3d n = Normalize(pos); 
	//beta angle
	double beta = acos(n.z);
	beta = beta*(NV-1.0)/M_PI+0.5;
	int iy = (int)beta;
	if (iy<0) iy=0;
	else if (iy>=NV) iy=NV-1;
	//alpha angle
	double alpha = atan2(n.y, n.x);
	if (alpha<0) alpha+=2*M_PI;
	alpha = alpha/(2*M_PI)*(NU-1)+0.5;
	int ix = (int)alpha;
	if (ix<0) ix=0;
	else if (ix>=NU) ix=NU-1;

	return NU*iy + ix;
}

int DGCubeMap::_getCloestAnchorVertex(const Vector3d &pos, const AnchorVertexInfo *pVertex, const int nv) const
{
	const Vector3d n = Normalize(pos); 
	double maxdotval = -100000.0;
	int index=-1;
	for (int i=0; i<nv; i++){
		double dotval = DotProd(n, pVertex[i].m_dir);
		if (maxdotval<dotval){
			maxdotval = dotval, index = i;
		}
	}
	return index;
}

DGCubeMap::DGCubeMap(void): 
	IMAGEWIDTH(65), 
	W05((IMAGEWIDTH-1)*0.5), 
	W05W105(W05*(1.0f+1.0f/(IMAGEWIDTH-1)))
{
	if ((IMAGEWIDTH&0x1)!=1){
		printf("Need odd CUBE MAP image size!\n");
		exit(0);
	}
	m_pImage[0]=m_pImage[1]=m_pImage[2]=NULL;
	m_pImage[3]=m_pImage[4]=m_pImage[5]=NULL;
}

DGCubeMap::~DGCubeMap(void)
{
	SafeDeleteArray(m_pImage[0]);
}

void DGCubeMap::initCubeMap(const AnchorVertexInfo *pVertex, const int NU, const int NV)
{
	SafeDeleteArray(m_pImage[0]);
	const int imgsize = ((IMAGEWIDTH * IMAGEWIDTH+4)>>2)<<2;
	m_pImage[0]=new unsigned int[imgsize*6];
	assert(m_pImage[0]!=NULL);
	m_pImage[1] = m_pImage[0] + imgsize;
	m_pImage[2] = m_pImage[1] + imgsize;
	m_pImage[3] = m_pImage[2] + imgsize;
	m_pImage[4] = m_pImage[3] + imgsize;
	m_pImage[5] = m_pImage[4] + imgsize;
	for (int k=0; k<6; k++){
		unsigned int *pImage = m_pImage[k];
		int pixelpos = 0;
		for (int j=0; j<IMAGEWIDTH; j++){
			for (int i=0; i<IMAGEWIDTH; i++, pixelpos++){
				const Vector3d pos = _getWorldPos(i, j, k);
				pImage[pixelpos] = _getCloestAnchorVertex(pos, pVertex, NU, NV);
			}
		}
	}
}

void DGCubeMap::testFunc(void)
{
	Vector3d pos = _getWorldPos(32, 32, 1);
}

//==================================================================
static void computeDeformationGradientOnSphere(const double alpha, const double beta, const int n, double3x3 &F)
{
	Vector3d force;
	CMeMaterialProperty mtl;
	CShearElement e(Vector3d(0, 0, 1), 1.0);
	for (int i=1; i<=n; i++){
		Vector3d p;
		const double K = ((double)i)/n;
		const double a = alpha;
		const double b = beta * K;
		const double Rxy = sin(b);
		p.x = cos(a)*Rxy;
		p.y = sin(a)*Rxy;
		p.z = cos(b);
		e.computeNodalForce(p, mtl, force, NULL);
	}

	F = *(e.getDeformationGradient());
}

inline Vector3d midpoint_slerp(const Vector3d& a, const Vector3d &b){Vector3d n = (a+b)*0.5; n.normalize(); return n;}

void SphericalQuadTree::exportMesh(const char *fname)
{
	FILE *fp = fopen(fname, _WA_);
	const int nv = m_vertices.size();
	const int ntri = m_leaves.size();
	fprintf(fp, "%d %d\n", nv, ntri);
	fprintf(fp, "1\n");
	int i;
	for (i=0; i<nv; i++){
		Vector3d v = m_vertices[i].m_dir;
		fprintf(fp, "%lg %lg %lg\n", v.x, v.y, v.z);
	}
	for (i=0; i<ntri; i++){
		Vector3s t = m_leaves[i].m_tri;
		fprintf(fp, "%d %d %d\n", (int)t.x, (int)t.y, (int)t.z);
	}
	fclose(fp);
}

SphericalQuadTree::SphericalQuadTree(const int ilevel):
	NORMAL_DOTPROD_TOL((int)(0.995*NORMAL_SCALE))
{
	const Vector3d XPLUS(1,  0,  0);	//  X 
	const Vector3d XMIN(-1,  0,  0);	// -X 
	const Vector3d YPLUS(0,  1,  0);	//  Y 
	const Vector3d YMIN(0, -1,  0);		// -Y 
	const Vector3d ZPLUS(0,  0,  1);	//  Z 
	const Vector3d ZMIN(0,  0, -1);		// -Z 
	m_nMaxLevel = ilevel;

	{//init the buffers
	const int nv = 3*(1<<ilevel)*8+80;
	m_vertices.resize(nv); 
	m_vertices.clear();

	const int nnodes = (1<<ilevel) * 8;
	m_nodes.resize(nnodes); m_nodes.clear();

	const int nleaves = (1<<ilevel) * 8;
	m_leaves.resize(nleaves); m_leaves.clear();
	}

	//init the first 8 triangles
	double3x3 I0; I0.setIdentityMatrix();
	ZPLUSid = m_vertices.size();
	m_vertices.push_back(AnchorVertexInfo(ZPLUS, I0));	//IMPORTANT: this is the first vertex
	ZMINid = m_vertices.size();
	m_vertices.push_back(AnchorVertexInfo(ZMIN, I0));
	XPLUSid = m_vertices.size();
	m_vertices.push_back(AnchorVertexInfo(XPLUS, I0));
	XMINid = m_vertices.size();
	m_vertices.push_back(AnchorVertexInfo(XMIN, I0));
	YPLUSid = m_vertices.size();
	m_vertices.push_back(AnchorVertexInfo(YPLUS, I0));
	YMINid = m_vertices.size();
	m_vertices.push_back(AnchorVertexInfo(YMIN, I0));

	NoneLeafNode t0; 
	const int tid0 = m_nodes.size(); m_nodes.push_back(t0);
	const int tid1 = m_nodes.size(); m_nodes.push_back(t0);
	const int tid2 = m_nodes.size(); m_nodes.push_back(t0);
	const int tid3 = m_nodes.size(); m_nodes.push_back(t0);
	const int tid4 = m_nodes.size(); m_nodes.push_back(t0);
	const int tid5 = m_nodes.size(); m_nodes.push_back(t0);
	const int tid6 = m_nodes.size(); m_nodes.push_back(t0);
	const int tid7 = m_nodes.size(); m_nodes.push_back(t0);
	
	const int level=0;
	const int maxlevel = ilevel;
	//half plane +Z
	_buildTree(tid7, XPLUS, XPLUSid, YPLUS, YPLUSid, ZPLUS, ZPLUSid, level, maxlevel);
	_buildTree(tid6, ZPLUS, ZPLUSid, YPLUS, YPLUSid, XMIN, XMINid, level, maxlevel);
	_buildTree(tid5, XPLUS, XPLUSid, ZPLUS, ZPLUSid, YMIN, YMINid, level, maxlevel);
	_buildTree(tid4, ZPLUS, ZPLUSid, XMIN, XMINid, YMIN, YMINid, level, maxlevel);
	//half plane -Z
	_buildTree(tid3, YPLUS, YPLUSid, XPLUS, XPLUSid, ZMIN, ZMINid, level, maxlevel);
	_buildTree(tid2, YPLUS, YPLUSid, ZMIN, ZMINid, XMIN, XMINid, level, maxlevel);
	_buildTree(tid1, XPLUS, XPLUSid, YMIN, YMINid, ZMIN, ZMINid, level, maxlevel);
	_buildTree(tid0, XMIN, XMINid, ZMIN, ZMINid, YMIN, YMINid, level, maxlevel);

	//now, set the DG
	_initDeformationTensors();
}

inline Vector3d tri_norm(const Vector3d& a, const Vector3d &b, const Vector3d &c)
{
	Vector3d n = (a+b+c)/3.0; n.normalize();
	return n;
}

void SphericalQuadTree::_buildTree(const int nodeid, 
		const Vector3d &p0, const int i0, const Vector3d &p1, const int i1, const Vector3d &p2, const int i2, 
		const int level, const int maxlevel)
{
	if (level>=maxlevel){
		m_leaves[nodeid] = LeafNode(i0, i1, i2);
		return;
	}
	Vector3d n[4];
	short int t[4]; 
	//allocate nodes or leaves
	if (level<maxlevel-1){
		NoneLeafNode _tt;
		t[0] = m_nodes.size(); m_nodes.push_back(_tt);
		t[1] = m_nodes.size(); m_nodes.push_back(_tt);
		t[2] = m_nodes.size(); m_nodes.push_back(_tt);
		t[3] = m_nodes.size(); m_nodes.push_back(_tt);
	}
	else{
		LeafNode _tt;
		t[0] = m_leaves.size(); m_leaves.push_back(_tt);
		t[1] = m_leaves.size(); m_leaves.push_back(_tt);
		t[2] = m_leaves.size(); m_leaves.push_back(_tt);
		t[3] = m_leaves.size(); m_leaves.push_back(_tt);
	}
	//construct the information for node itself
	const Vector3d q0 = midpoint_slerp(p0, p1);
	const Vector3d q1 = midpoint_slerp(p1, p2);
	const Vector3d q2 = midpoint_slerp(p2, p0);
	const int j0 = _getVertexIndex(q0);
	const int j1 = _getVertexIndex(q1);
	const int j2 = _getVertexIndex(q2);
	n[0] = tri_norm(p0, q0, q2);
	n[1] = tri_norm(q0, p1, q1);
	n[2] = tri_norm(q0, q1, q2);
	n[3] = tri_norm(q2, q1, p2);
	m_nodes[nodeid] = NoneLeafNode(n, t);
	//build child nodes
	const int level1 = level+1;
	_buildTree(t[0], p0, i0, q0, j0, q2, j2, level1, maxlevel); 
	_buildTree(t[1], q0, j0, p1, i1, q1, j1, level1, maxlevel); 
	_buildTree(t[2], q0, j0, q1, j1, q2, j2, level1, maxlevel); 
	_buildTree(t[3], q2, j2, q1, j1, p2, i2, level1, maxlevel); 
}

void SphericalQuadTree::_propergateDeformationTensor(const int passid, const int i, const int j)
{
	AnchorVertexInfo *pi = &m_vertices[i];
	AnchorVertexInfo *pj = &m_vertices[j];
	if (pi->m_level==pj->m_level) return;
	if (pi->m_level>passid || pj->m_level>passid) return;
	if (pi->m_level<pj->m_level){
		AnchorVertexInfo* _t = pi;
		pi = pj; pj = _t;
	}
	//compute the DG
	Vector3d force;
	CMeMaterialProperty mtl;
	CShearElement e(pi->m_dir, 1.0);
	e.setDeformationGradient(pi->m_F);
	const Vector3d midp = midpoint_slerp(pi->m_dir, pj->m_dir);
	e.computeNodalForce(midp, mtl, force, NULL);
	e.computeNodalForce(pj->m_dir, mtl, force, NULL);
	//save the DG
	Tensor3x3 *dg = e.getDeformationGradient();
	if (pi->m_level == (pj->m_level+1)){
		pj->m_F += *dg;
		pj->m_count++;
	}
	else{
		pj->m_F = *dg;
		pj->m_count=1;
		pj->m_level = pi->m_level -1;
	}
}

void SphericalQuadTree::_initDeformationTensors(void)
{
	const int N=200;
	{//prepare the 6 seed points
	const int DIV = 1<<(m_nMaxLevel+1);
	double3x3 F, F1, F2, F3, F4; 
	F.setIdentityMatrix();
	m_vertices[ZPLUSid].m_count = 1;
	m_vertices[ZPLUSid].m_level = N;
	m_vertices[ZPLUSid].m_F = F;

	computeDeformationGradientOnSphere(0, PI, 2*DIV, F1);
	computeDeformationGradientOnSphere(PI*0.5, PI, 2*DIV, F2);
	computeDeformationGradientOnSphere(PI, PI, 2*DIV, F3);
	computeDeformationGradientOnSphere(-PI*0.5, PI, 2*DIV, F4);
	F = (F1+F2+F3+F4)*0.25;
	m_vertices[ZMINid].m_count = 1;
	m_vertices[ZMINid].m_level = N;
	m_vertices[ZMINid].m_F = F;

	computeDeformationGradientOnSphere(0, PI*0.5, DIV, F);
	m_vertices[XPLUSid].m_count = 1;
	m_vertices[XPLUSid].m_level = N;
	m_vertices[XPLUSid].m_F = F;

	computeDeformationGradientOnSphere(PI, PI*0.5, DIV, F);
	m_vertices[XMINid].m_count = 1;
	m_vertices[XMINid].m_level = N;
	m_vertices[XMINid].m_F = F;

	computeDeformationGradientOnSphere(PI*0.5, PI*0.5, DIV, F);
	m_vertices[YPLUSid].m_count = 1;
	m_vertices[YPLUSid].m_level = N;
	m_vertices[YPLUSid].m_F = F;

	computeDeformationGradientOnSphere(-PI*0.5, PI*0.5, DIV, F);
	m_vertices[YMINid].m_count = 1;
	m_vertices[YMINid].m_level = N;
	m_vertices[YMINid].m_F = F;
	}

	int i, j;
	const int ntri = m_leaves.size();
	const int nv = m_vertices.size();
	for (i=N; i>0; i--){
		for (j=0; j<ntri; j++){
			const Vector3s tri = m_leaves[j].m_tri;
			_propergateDeformationTensor(i, tri.x, tri.y);
			_propergateDeformationTensor(i, tri.x, tri.z);
			_propergateDeformationTensor(i, tri.y, tri.z);
		}
		bool finish = true;
		for (j=0; j<nv; j++){
			if (m_vertices[j].m_count==0 || m_vertices[j].m_level==0) 
				finish = false;
		}
		if (finish) break;
	}
	if (i==0){
		printf("Propergation Alert: iteration not enough or data error!\n");
		exit(0);
	}
	for (i=0; i<nv; i++){
		AnchorVertexInfo *p = &m_vertices[i];
		if (p->m_count>1){
			p->m_F *= (1.0/p->m_count);
		}
	}
}

//=============================================================
CGyrodLookupTable::CGyrodLookupTable(const int ndiv)
{
	ZAccurateTimer timer;
	timer.start();
	m_nv = ndiv+1; 
	m_nu = ndiv*2+1;
	const int nvsize = m_nu * m_nv;
	m_Scale = ndiv/M_PI;
	m_pElement = NULL;
	_genLookupTable();
	//use cube map for accelaration
	m_pCubeMap = new DGCubeMap();
	m_pCubeMap->initCubeMap(m_pElement, m_nu, m_nv);
	timer.stop();
	const double tm = timer.getTimeInMiliSecond();
	printf("Gen GYROD lookup table time is %lgms.\n", tm);
}

CGyrodLookupTable::~CGyrodLookupTable(void)
{
	SafeDeleteArray(m_pElement);
	SafeDelete(m_pCubeMap);
}

void CGyrodLookupTable::_genLookupTable(void)
{
	SafeDeleteArray(m_pElement);
	const int nsize = m_nu*m_nv;
	m_pElement = new CLookupTableElement[nsize];	
	assert(m_pElement!=NULL);

	CMeMaterialProperty matril;
	matril.setDamping(0,0);
	const double KU = 2.0*PI/(m_nu-1);
	const double KV = PI/(m_nv-1);
	for (int u=0; u<m_nu; u++){
		CShearElement e(Vector3d(0,0,1), 1);
		const double alpha = u*KU;
		double cosalpha = cos(alpha);
		double sinalpha = sin(alpha);
		if (u==(m_nu-1)*0.25)
			cosalpha = 0,  sinalpha = 1;
		else if (u==(m_nu-1)*0.50)
			cosalpha = -1,  sinalpha = 0;
		else if (u==(m_nu-1)*0.75)
			cosalpha = 0,  sinalpha = -1;
		else if (u==(m_nu-1))
			cosalpha = 1,  sinalpha = 0;
		for (int v=0; v<m_nv; v++){
			const double beta = v*KV;
			double sinbeta = sin(beta);
			double cosbeta = cos(beta);
			if (v==(m_nv-1)*0.50)
				cosbeta = 0,  sinbeta = 1;
			else if (v==(m_nv-1))
				cosbeta = 1,  sinbeta = 0;
			Vector3d force, dir;
			dir.x = cosalpha*sinbeta;
			dir.y = sinalpha*sinbeta;
			dir.z = cosbeta;
			const Vector3d olddir = e.getLocalPosition();
			Vector3d middir = midpoint_slerp(dir, olddir); 
			e.computeNodalForce(middir, matril, force, NULL);
			e.computeNodalForce(dir, matril, force, NULL);
			Tensor3x3 *t = e.getDeformationGradient();
			const int ipos = this->_getLookupTableElementIndex(u, v);
			m_pElement[ipos] = CLookupTableElement(dir, *t);
		}
	}
}
