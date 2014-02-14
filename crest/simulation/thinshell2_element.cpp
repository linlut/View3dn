//FILE: thinsehll2_element.cpp
#include <set>
#include <geomath.h>
#include <ztime.h>
#include "thinshell2_element.h"


static inline Vector3d _getVertexUsingStride(const Vector3d *parray, const int n, const int stride)
{
	const char *p = (const char *)parray;
	p += stride*n;
	return *((const Vector3d *)p);
}

inline void _searchNodeInPolygon(
	const int nodeid, const int *poly, const int nv_per_poly, set<int>& nodeset)
{
	int i, buf[20];
	for (i=0; i<nv_per_poly; i++) buf[i]=poly[i];
	for (i=0; i<nv_per_poly; i++) buf[i+nv_per_poly]=buf[i];
	for (i=1; i<=nv_per_poly; i++)
		if (buf[i]==nodeid) break;
	ASSERT0(i<=nv_per_poly);
	nodeset.insert(buf[i+1]);
	nodeset.insert(buf[i-1]);
}

void CThinshell2Element::init(const int isstatic, 
		const CMeMaterialProperty& mtl,	
		const int nodeid,						//the center vertex id
		const Vector3d *p,						//vertex position buffer
		const Vector3d *wpnorm,					//weighted polygon normal array
		const int *surfpoly,					//buffer of the whole surface polygons
		const int nv_per_elm,					//number of vertices for each surface polygon
		const int *polyfanid,					//the polyfan around the center vertex, input IDs only
		const int n1RingPoly,					//valence of the center vertex, also length of poylfanid
		const double& thickness)
{
	double plyvolume[MAX_NODE_VALENCE+1], edgevolume[MAX_NODE_VALENCE+1];
	int i, k;

	m_n1RingPoly = n1RingPoly;
	m_nCenterID = nodeid;
	for (i=0; i<n1RingPoly; i++) m_n1RingPolyID[i]=polyfanid[i];

	//Find all the fan polygons, store in tri buffer
	set<int> nodeset; nodeset.clear();
	for (i=0; i<n1RingPoly; i++){
		const int plyid = polyfanid[i];
		const int *ppoly = &surfpoly[plyid*nv_per_elm];
		_searchNodeInPolygon(nodeid, ppoly, nv_per_elm, nodeset);
		if (nv_per_elm==3){
			Vector3i tri(ppoly[0], ppoly[1], ppoly[2]);
			plyvolume[i] = triangle_area(p[tri.x], p[tri.y], p[tri.z]);
		}
		else{
			ASSERT0(nv_per_elm==4);
			Vector4i tri(ppoly[0], ppoly[1], ppoly[2], ppoly[3]);
			plyvolume[i] = quad_area(p[tri.x], p[tri.y], p[tri.z], p[tri.w]);
		}
	}

	//use the fan polygons to find the truss vertices
	set<int>::iterator itr = nodeset.begin();
	k = 0;
	while(itr!=nodeset.end()){
		m_nNodeID[k] = *itr;
		itr++, k++;
	}
	m_nRod = k;
	assert(m_nRod>0);
	if (m_nRod>MAX_NODE_VALENCE){
		printf("ALERT: node valence buffer is too small, increase to %d!!\n", m_nRod);
		m_nRod = MAX_NODE_VALENCE;
	}

	//find truss edge volume
	for (i=0; i<m_n1RingPoly; i++) plyvolume[i]*= thickness/nv_per_elm;
	for (i=0; i<m_nRod; i++) edgevolume[i]=0;
	for (i=0; i<m_n1RingPoly; i++){
		const double vol = plyvolume[i];
		const int plyid = polyfanid[i];
		const int *ppoly = &surfpoly[plyid*nv_per_elm];
		for (int j=0; j<nv_per_elm; j++){
			const int id = ppoly[j];
			for (k=0; k<m_nRod; k++){ 
				if (m_nNodeID[k]==id) 
					break;
			}
			if (k<m_nRod)
				edgevolume[k]+=vol;
		}
	}

	_initShearElements(isstatic, mtl, p, wpnorm, edgevolume);
}

static inline Vector3d
_computeAccurateVertexNormal(const Vector3d p[], const int stride, const int centerid, const Vector2i tri[], const int nvalence)
{
	Vector3d N(0,0,0), n;
	int i;
	const Vector3d p0 = _getVertexUsingStride(p, centerid, stride);
	for (i=0; i<nvalence; i++){
		const int x = tri[i].x;
		const int y = tri[i].y;
		const Vector3d px = _getVertexUsingStride(p, x, stride);
		const Vector3d py = _getVertexUsingStride(p, y, stride);
		compute_triangle_areanormal(p0, px, py, n);
		N += n;
	}
	N.normalize();
	return N;
}

static inline Vector3d 
_computeAccurateVertexNormal2(const Vector3d areanorm[], const int *polyid, const int n)
{
	ASSERT0(n>=1);
	Vector3d N0 = areanorm[polyid[0]];
	for (int i=1; i<n; i++){
		const int j=polyid[i];
		N0+=areanorm[j];
	}
	N0.normalize();
	return N0;
}

inline void CThinshell2Element::_computeLocalTransformMatrix2Tagent(
	const Vector3d p[], const int stride, const Vector3d norm[], double3x3& mat)
{
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	Vector3d p0 = _getVertexUsingStride(p, m_nCenterID, stride);
	const int ii = 0;
	Vector3d p1 = _getVertexUsingStride(p, m_nNodeID[ii], stride);
	const int len = m_nRod >> 1;		//div by 2
	for (int i=1; i<len; i++){
		const int j = m_nNodeID[i]; 
		p1 += _getVertexUsingStride(p, j, stride);
	}

	//Z = _computeAccurateVertexNormal(p, stride, m_nCenterID, m_1ringTri, m_n1RingPoly);
	Z = _computeAccurateVertexNormal2(norm, m_n1RingPolyID, m_n1RingPoly);
	X = p1 - p0; 
	Y = CrossProd(Z, X); Y.normalize();
	X = CrossProd(Y, Z);
}

//shear element composed of two triangles
void CThinshell2Element::_initShearElements(
		const int isStatic, 
		const CMeMaterialProperty& mtl,	
		const Vector3d p[], const Vector3d wpnorm[], const double trussvolumes[])
{
	//compute volume for each triangle, each truss edge
	const int stride = sizeof(Vector3d);

	//construct the world/local transform matrix for the two triangles
	double3x3 rot; rot.setIdentityMatrix();
	if (!isStatic){
		_computeLocalTransformMatrix2Tagent(p, stride, wpnorm, rot);
	}

	const Vector3d p0 = p[m_nCenterID];
	for (int i=0; i<m_nRod; i++){
		const int j = m_nNodeID[i];
		const Vector3d pj = p[j];
		const double trussvol = trussvolumes[i];
		m_shearelm[i].init(mtl, pj, p0, rot, trussvol);
	}
}

void CThinshell2Element::computeForce(
		const int isStatic,					//static simulation or dynamic simu	
		const Vector3d p[],					//nodes' current positions
		const int stride,					//consider nodes are packed in an array which are not continious
		const Vector3d pnormal[],			//an input normal array for the whole mesh, not only the element
		const CMeMaterialProperty &mtl,		//material
		Vector3d F[],						//force array
		double3x3 *ppJacobian[])			//jacobian array
{
	const Vector3d p0 = _getVertexUsingStride(p, m_nCenterID, stride);
	Vector3d* F1 = &F[1];
	double3x3 **ppJacobian1 = ppJacobian+1;	
	if (isStatic){
		for (int i=0; i<m_nRod; i++){
			const int j = m_nNodeID[i];
			const Vector3d pj = _getVertexUsingStride(p, j, stride);
			m_shearelm[i].computeNodalForce(pj, p0, mtl, F1[i], ppJacobian1[i]);
		}
	}
	else{
		//gen. the world/local and local/world transformation matrices
		double3x3 rotmat;
		_computeLocalTransformMatrix2Tagent(p, stride, pnormal, rotmat);
		//compute local forces for each shear element
		for (int i=0; i<m_nRod; i++){
			const int j = m_nNodeID[i];
			const Vector3d pj = _getVertexUsingStride(p, j, stride);
			m_shearelm[i].computeNodalForce(pj, p0, rotmat, mtl, F1[i], ppJacobian1[i]);
		}
	}

	//accumulate forces on the nodes
	int i;
	for (i=1, F[0]=-F1[0]; i<m_nRod; i++) F[0]-=F1[i];
}


//=====================================================================
static int test_func(void)
{/*
    CMeMaterialProperty mtl;
    const int timerid = 0;
    const int N=20;
    CThinshell2Element shell;
	const int nv_per_elm=3;

	Vector3d pVertex[]={
		Vector3d(0, 0, 0),
		Vector3d(1, 0, 0),
		Vector3d(1, 1, 0),
		Vector3d(0, 1, 0),
		Vector3d(0.5, 0.5, 0)
	};
    Vector3d F[20];
    double3x3 *ppJacobian[20]={NULL, NULL, NULL, NULL, NULL, NULL};
    double t=1e-3;
    int tri[]={
		0,1,4,
		4, 1, 2,
		3, 4, 2,
		3, 0, 4
	};
	const int nValence=4;
	int polyfanid[nValence]={0,1,2,3};
	const int nodeid = 4;
    shell.init(0, 4, pVertex, tri, nv_per_elm, polyfanid, nValence, t);
	//return 1;

    startFastTimer(timerid);
	const int stride = sizeof(Vector3d);
    for (int i=0; i<N; i++){
		const int j=0;
        const double z = 1.0*i/N;
        pVertex[4].z = z;
        shell.computeForce(0, pVertex, stride, mtl, F, ppJacobian);
        printf("Loop %3d, Force is %8lg %8lg %8lg\n", i, F[j].x, F[j].y, F[j].z);
    }
    stopFastTimer(timerid);
    reportTimeDifference(timerid, "Shell elm run time:");
*/
    return 1;
}
static int ntest = test_func();


