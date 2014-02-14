//marchingtet.cpp

#include "tetraobj.h"
#include "dynamicmesh.h"

//=======Functions used for storage=================
CDynamicMesh *pDynMesh=NULL;


static inline int AddVertexToBuffer(const Vector3f& v, const Vector3f& tex, 
	const Vector3f& norm=Vector3f(0,0,0), const int attr=0, const int bid=0)
{
	ASSERT0(pDynMesh!=NULL);
	CGeneratedVertex gvertex(v, tex, norm, attr);
	int n;

	if (bid==0){
		n = pDynMesh->m_FractVertex.GetSize();
		pDynMesh->m_FractVertex.Add(gvertex);
	}
	else{
		n = pDynMesh->m_FractVertex2.GetSize();
		pDynMesh->m_FractVertex2.Add(gvertex);
	}
	return n; 
}


static inline int AddTriangleToBuffer(const Vector3i& tri, const int bid=0)
{
	ASSERT0(pDynMesh!=NULL);
	int n;
	if (bid==0){
		n = pDynMesh->m_FractTri.GetSize();
		pDynMesh->m_FractTri.Add(tri);
	}
	else{
		n = pDynMesh->m_FractTri2.GetSize();
		pDynMesh->m_FractTri2.Add(tri);
	}
	return n; 
}


void _CheckBuffer(const int bid=0)
{
	int nv, ntri;
	Vector3i *ptri;

	if (bid==0){
		ntri = pDynMesh->m_FractTri.GetSize();
		nv = pDynMesh->m_FractVertex.GetSize();
		ptri = pDynMesh->m_FractTri.GetBuffer();
	}
	else{
		ntri = pDynMesh->m_FractTri2.GetSize();
		nv = pDynMesh->m_FractVertex2.GetSize();
		ptri = pDynMesh->m_FractTri2.GetBuffer();
	}
	for (int i=0; i<ntri; i++){
		const int v0=ptri[i].x;
		const int v1=ptri[i].y;
		const int v2=ptri[i].z;
		if (v0>=nv || v1>=nv ||v2>=nv){
			printf("BUffer wrong!\n");
		}
	}
}

//====================================================
static inline 
Vector3i * _bsearchTriangleArray(const int v0, const int v1, const int v2, 
								Vector3i* pBoundTri, const int nBoundTri)
{
	Vector3i tri(v0, v1, v2); 
	tri.bubbleSort();
	//use binary search on the already sorted array
	Vector3i*p=(Vector3i*)bsearch(&tri, pBoundTri, nBoundTri, sizeof(Vector3i), _vec3i_compare);
	return p;
}


static inline 
void _computeLocalFrame(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, 
						Vector3f& X, Vector3f& Y, Vector3f& Z, float& avg_edgelen) 
{
	const Vector3f d0 = v1 - v0;
	const Vector3f d1 = v2 - v1;
	const Vector3f d2 = v0 - v2;

	Z = CrossProd(d0, d1);
	X = d0;
	Y = CrossProd(Z, X);

	avg_edgelen = DotProd(d0, d0) + DotProd(d1, d1) + DotProd(d2, d2);
	X.normalize();
	Y.normalize();
	Z.normalize();
}

static inline
void _verifyTheTransform(const int v0, const Vector3f& p0, const int v1, const Vector3f& p1, CVertexInfo *pVertInfo)
{
	float r1 = Distance2(p0, p1);
	float r2 = Distance2(pVertInfo[v0].m_vInitPosition, pVertInfo[v1].m_vInitPosition);
	float k;
	if (r1<r2) k = r2 /r1;
	else k=r1/r2;
	if (k>15){
		printf("Something wrong!!\n");
	}
}

static inline 
Vector3f _computeVirtualCoorForV0(const int v0, const int v1, const int v2, const int v3, CVertexInfo *pVertInfo)
{
	const Vector3f p0 = pVertInfo[v0].m_vInitPosition;
	const Vector3f p1 = pVertInfo[v1].m_vInitPosition;
	const Vector3f p2 = pVertInfo[v2].m_vInitPosition;
	const Vector3f p3 = pVertInfo[v3].m_vInitPosition;
	const Vector3f base0=(p1+p2+p3)*0.3333333f;
	Vector3f d = (p0-base0);

	Vector3f X0, Y0, Z0, X1, Y1, Z1;
	float r0, r1;
	_computeLocalFrame(p1-base0, p2-base0, p3-base0, X0, Y0, Z0, r0);
	const float dx = DotProd(d, X0);
	const float dy = DotProd(d, Y0);
	const float dz = DotProd(d, Z0);

	Vector3f q0; 
	const Vector3f q1 = pVertInfo[v1].m_vCurrPosition;
	const Vector3f q2 = pVertInfo[v2].m_vCurrPosition;
	const Vector3f q3 = pVertInfo[v3].m_vCurrPosition;
	const Vector3f base1 = (q1+q2+q3)*0.3333333f;
	_computeLocalFrame(q1-base1, q2-base1, q3-base1, X1, Y1, Z1, r1);
	
	float ratio = (r1/r0), r2=ratio;
	ratio = sqrt(ratio);
	if (ratio>1.2f) ratio=1.2f;
	q0 = base1 + ratio*((X1*dx) + (Y1*dy) + (Z1*dz));
	return q0;
}


static inline 
void AddBoundaryTrianglesCase1_1(
	const int v0, const int v1, const int v2, const int v3,  
	const Vector3f& p01, const Vector3f& p02, const Vector3f& p03,
	const Vector3f& t01, const Vector3f& t02, const Vector3f& t03,
	CVertexInfo *pVertInfo) 
{
	const Vector3f& p0 = pVertInfo[v0].m_vCurrPosition;
	const Vector3f& t0 = pVertInfo[v0].m_vInitPosition;

	const int ip0= AddVertexToBuffer(p0, t0);
	const int ip01= AddVertexToBuffer(p01, t01);
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);

	if (TriangleOnStaticBoundary(v0, v1, v2, pVertInfo))
		AddTriangleToBuffer(Vector3i(ip0, ip01, ip02));
	
	if (TriangleOnStaticBoundary(v0, v2, v3, pVertInfo))
		AddTriangleToBuffer(Vector3i(ip0, ip02, ip03));
	
	if (TriangleOnStaticBoundary(v0, v1, v3, pVertInfo))
		AddTriangleToBuffer(Vector3i(ip0, ip03, ip01));	
}


static inline 
void AddBoundaryTrianglesCase1_2(
				const int v0, const int v1, const int v2, const int v3, 
				const Vector3f& p01, const Vector3f& p02, const Vector3f& p03, //intps
				const Vector3f& t01, const Vector3f& t02, const Vector3f& t03, //intps' texture coor.
				CVertexInfo *pVertInfo)
{
	const Vector3f& p1 = pVertInfo[v1].m_vCurrPosition;
	const Vector3f& p2 = pVertInfo[v2].m_vCurrPosition;
	const Vector3f& p3 = pVertInfo[v3].m_vCurrPosition;
	const Vector3f& t1 = pVertInfo[v1].m_vInitPosition;
	const Vector3f& t2 = pVertInfo[v2].m_vInitPosition;
	const Vector3f& t3 = pVertInfo[v3].m_vInitPosition;

	const int ip1= AddVertexToBuffer(p1, t1);
	const int ip2= AddVertexToBuffer(p2, t2);
	const int ip3= AddVertexToBuffer(p3, t3);
	const int ip01= AddVertexToBuffer(p01, t01);
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);

	const bool r0 = pVertInfo[v0].getHasFlatCutFlag();
	const bool r1 = pVertInfo[v1].getHasFlatCutFlag();
	const bool r2 = pVertInfo[v2].getHasFlatCutFlag();
	const bool r3 = pVertInfo[v3].getHasFlatCutFlag();

	if (r1||r2||TriangleOnStaticBoundary(v0, v1, v2, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip01, ip1, ip02));	
		AddTriangleToBuffer(Vector3i(ip02, ip1, ip2));	
	}
	if (r1||r3||TriangleOnStaticBoundary(v0, v1, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip01, ip03, ip3));	
		AddTriangleToBuffer(Vector3i(ip01, ip3, ip1));	
	}
	if (r2||r3||TriangleOnStaticBoundary(v0, v2, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip02, ip2, ip3));	
		AddTriangleToBuffer(Vector3i(ip02, ip3, ip03));	
	}
}



void CMarchingTet::_genPatchForFirstCut(const int tm, 
		const int v0, const int v1, const int v2, const int v3, 
		const Vector3f& p0, const Vector3f& p1, const Vector3f& p2, const Vector3f& p3, 
		CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo)
{
	//perform edge split
	const int eidx01 = _getEdgePointerByVertexIDs(v0, v1);
	const int eidx02 = _getEdgePointerByVertexIDs(v0, v2);
	const int eidx03 = _getEdgePointerByVertexIDs(v0, v3);
	pSplitEdges[eidx01].PerformSplit(tm, v0, p0, v1, p1, pVertInfo);
	pSplitEdges[eidx02].PerformSplit(tm, v0, p0, v2, p2, pVertInfo);
	pSplitEdges[eidx03].PerformSplit(tm, v0, p0, v3, p3, pVertInfo);

	//get cut points, remember to inverse the normal direction;
	const Vector3f p01 = pSplitEdges[eidx01].GetSplitPosition(v0, pVertInfo);
	const Vector3f p02 = pSplitEdges[eidx02].GetSplitPosition(v0, pVertInfo);
	const Vector3f p03 = pSplitEdges[eidx03].GetSplitPosition(v0, pVertInfo);
	const Vector3f t01 = pSplitEdges[eidx01].GetSplitTextCoor(pVertInfo);
	const Vector3f t02 = pSplitEdges[eidx02].GetSplitTextCoor(pVertInfo);
	const Vector3f t03 = pSplitEdges[eidx03].GetSplitTextCoor(pVertInfo);

	const int ip01= AddVertexToBuffer(p01, t01);
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);
	AddTriangleToBuffer(Vector3i(ip01, ip03, ip02));	

	//append boundary triangles;
	AddBoundaryTrianglesCase1_1(v0, v1, v2, v3, p01, p02, p03, t01, t02, t03, pVertInfo);
}


//======================CASE: CUT ONE===============================

void CMarchingTet::_extractSurface1(const int tm, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo)
{
	int v0, v1, v2, v3, eidx01, eidx02, eidx03;
	Vector3f p0, p1, p2, p3, p01, p02, p03, q0, q1, q2, q3;

	//get the vertices and edges;
	_getVerticesAndEdgesByExtractionSequence(0, pVertInfo, v0, v1, v2, v3, eidx01, eidx02, eidx03);
	p0 = _computeVirtualCoorForV0(v0, v1, v2, v3, pVertInfo);
	p1 = pVertInfo[v1].m_vCurrPosition;
	p2 = pVertInfo[v2].m_vCurrPosition;
	p3 = pVertInfo[v3].m_vCurrPosition;
	q0=p0, q1=p1, q2=p2, q3=p3;

	const bool pflag = pVertInfo[v0].getParticleFlag();
	if (!pflag){	//for non-particle vertex, use it's current position as basis
		const Vector3i* pBaseTri=_lsearchDynamicVertex(v0, pVertInfo);
		ASSERT0(pBaseTri!=NULL);
		p0 = pVertInfo[v0].m_vCurrPosition;
		p1 = _computeVirtualCoorForV0(v1, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p2 = _computeVirtualCoorForV0(v2, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p3 = _computeVirtualCoorForV0(v3, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
	}
	_genPatchForFirstCut(tm, v0, v1, v2, v3, p0, p1, p2, p3, pSplitEdges, pVertInfo);
	p0=q0, p1=q1, p2=q2, p3=q3;

	//split for the second part of the cut
	pSplitEdges[eidx01].PerformSplit(tm, v1, p1, v0, p0, pVertInfo);
	pSplitEdges[eidx02].PerformSplit(tm, v2, p2, v0, p0, pVertInfo);
	pSplitEdges[eidx03].PerformSplit(tm, v3, p3, v0, p0, pVertInfo);

	//add one opposite triangle 
	p01= pSplitEdges[eidx01].GetSplitPosition(v1, pVertInfo);
	p02= pSplitEdges[eidx02].GetSplitPosition(v2, pVertInfo);
	p03= pSplitEdges[eidx03].GetSplitPosition(v3, pVertInfo);
	const Vector3f t01= pSplitEdges[eidx01].GetSplitTextCoor(pVertInfo);
	const Vector3f t02= pSplitEdges[eidx02].GetSplitTextCoor(pVertInfo);
	const Vector3f t03= pSplitEdges[eidx03].GetSplitTextCoor(pVertInfo);
	const int ip01= AddVertexToBuffer(p01, t01);
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);
	AddTriangleToBuffer(Vector3i(ip01, ip02, ip03));	

	//append boundary triangles;
	AddBoundaryTrianglesCase1_2(v0, v1, v2, v3, p01, p02, p03, t01, t02, t03, pVertInfo);	
}

//======================CASE: CUT TWO===============================

static inline
void reOrderVertices(const int firstvid, int& v1, int& v2, int& v3, int &eidx01, int& eidx02, int& eidx03)
{
	const int v[3] = { v1, v2, v3};
	const int e[3] = { eidx01, eidx02, eidx03};
	int i;
	for (i=0; i<3; i++){
		if (v[i]==firstvid)
			break;
	}
	ASSERT0(i<3);
	v1 = v[i], eidx01 = e[i];
	int k = (i+1)%3; v2=v[k], eidx02=e[k]; 
	k = (i+2)%3;     v3=v[k], eidx03=e[k]; 
}


static inline
void addInterfaceSurface2_1(
	const Vector3f &p01, const Vector3f &p02, const Vector3f &p03, const Vector3f &p012, const Vector3f &p013, const Vector3f &p023, const Vector3f &p0123, 
	const Vector3f &t01, const Vector3f &t02, const Vector3f &t03, const Vector3f &t012, const Vector3f &t013, const Vector3f &t023, const Vector3f &t0123)
{	
	const int ip01= AddVertexToBuffer(p01, t01);
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);
	const int ip012= AddVertexToBuffer(p012, t012);
	const int ip013= AddVertexToBuffer(p013, t013);
	const int ip023= AddVertexToBuffer(p023, t023);
	const int ip0123= AddVertexToBuffer(p0123, t0123);

	AddTriangleToBuffer(Vector3i(ip01, ip0123, ip012));	
	AddTriangleToBuffer(Vector3i(ip01, ip013, ip0123));	

	AddTriangleToBuffer(Vector3i(ip0123, ip013, ip03));	
	AddTriangleToBuffer(Vector3i(ip0123, ip03, ip023));	

	AddTriangleToBuffer(Vector3i(ip0123, ip02, ip012));	
	AddTriangleToBuffer(Vector3i(ip0123, ip023, ip02));	
}

static inline
void addBoundarySurface2_1(
		 const int v0, const int v1, const int v2, const int v3, 
		 CVertexInfo *pVertInfo, 
		 const Vector3f &p0, const Vector3f &p01, const Vector3f &p02, const Vector3f &p03, 
		 const Vector3f &p012, const Vector3f &p013, const Vector3f &p023, const Vector3f &p0123,
		 const Vector3f &t0, const Vector3f &t01, const Vector3f &t02, const Vector3f &t03, 
		 const Vector3f &t012, const Vector3f &t013, const Vector3f &t023, const Vector3f &t0123)
{
	const int ip0= AddVertexToBuffer(p0, t0);
	const int ip01= AddVertexToBuffer(p01, t01);
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);
	const int ip012= AddVertexToBuffer(p012, t012);
	const int ip013= AddVertexToBuffer(p013, t013);
	const int ip023= AddVertexToBuffer(p023, t023);
	const int ip0123= AddVertexToBuffer(p0123, t0123);
	const bool r0 = pVertInfo[v0].getHasFlatCutFlag();

	if (r0||TriangleOnStaticBoundary(v0, v1, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip0, ip03, ip013));	
		AddTriangleToBuffer(Vector3i(ip0, ip013, ip01));	
	}

	if (r0||TriangleOnStaticBoundary(v0, v2, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip0, ip02, ip023));	
		AddTriangleToBuffer(Vector3i(ip0, ip023, ip03));	
	}

	if (r0||TriangleOnStaticBoundary(v0, v1, v2, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip0, ip01, ip012));	
		AddTriangleToBuffer(Vector3i(ip0, ip012, ip02));	
	}
}

static inline
void addInterfaceSurface2_2(
	const Vector3f &p02, const Vector3f &p03, const Vector3f &p12, const Vector3f &p13, 
	const Vector3f &p012, const Vector3f &p013, const Vector3f &p023, const Vector3f &p0123, 
	const Vector3f &t02, const Vector3f &t03, const Vector3f &t12, const Vector3f &t13, 
	const Vector3f &t012, const Vector3f &t013, const Vector3f &t023, const Vector3f &t0123)
{	
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);
	const int ip12= AddVertexToBuffer(p12, t12);
	const int ip13= AddVertexToBuffer(p13, t13);
	const int ip012= AddVertexToBuffer(p012, t012);
	const int ip013= AddVertexToBuffer(p013, t013);
	const int ip023= AddVertexToBuffer(p023, t023);
	const int ip0123= AddVertexToBuffer(p0123, t0123);

	AddTriangleToBuffer(Vector3i(ip0123, ip013, ip13));	
	AddTriangleToBuffer(Vector3i(ip0123, ip13, ip12));	
	AddTriangleToBuffer(Vector3i(ip0123, ip12, ip012));	

	AddTriangleToBuffer(Vector3i(ip0123, ip023, ip03));	
	AddTriangleToBuffer(Vector3i(ip0123, ip03, ip013));	

	AddTriangleToBuffer(Vector3i(ip0123, ip012, ip02));	
	AddTriangleToBuffer(Vector3i(ip0123, ip02, ip023));	
}

static inline
void addBoundarySurface2_2(
		 const int v0, const int v1, const int v2, const int v3, 
		 CVertexInfo *pVertInfo, 
		 const Vector3f &p2, const Vector3f &p3,
		 const Vector3f &p02, const Vector3f &p03, const Vector3f &p12, const Vector3f &p13, 
		 const Vector3f &p012, const Vector3f &p013, const Vector3f &p023, const Vector3f &p0123,
		 const Vector3f &t2, const Vector3f &t3,
		 const Vector3f &t02, const Vector3f &t03, const Vector3f &t12, const Vector3f &t13, 
		 const Vector3f &t012, const Vector3f &t013, const Vector3f &t023, const Vector3f &t0123)
{
	const int ip2= AddVertexToBuffer(p2, t2);
	const int ip3= AddVertexToBuffer(p3, t3);
	const int ip02= AddVertexToBuffer(p02, t02);
	const int ip03= AddVertexToBuffer(p03, t03);
	const int ip12= AddVertexToBuffer(p12, t12);
	const int ip13= AddVertexToBuffer(p13, t13);
	const int ip012= AddVertexToBuffer(p012, t012);
	const int ip013= AddVertexToBuffer(p013, t013);
	const int ip023= AddVertexToBuffer(p023, t023);
	const int ip0123= AddVertexToBuffer(p0123, t0123);
	const bool r0 = pVertInfo[v0].getHasFlatCutFlag();
	const bool r1 = pVertInfo[v1].getHasFlatCutFlag();
	const bool r2 = pVertInfo[v2].getHasFlatCutFlag();
	const bool r3 = pVertInfo[v3].getHasFlatCutFlag();

	if (r3||TriangleOnStaticBoundary(v0, v1, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip3, ip13, ip03));	
		AddTriangleToBuffer(Vector3i(ip03, ip13, ip013));	
	}

	if (r2||r3||TriangleOnStaticBoundary(v1, v2, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip3, ip2, ip12));	
		AddTriangleToBuffer(Vector3i(ip3, ip12, ip13));	
	}

	if (r2||TriangleOnStaticBoundary(v0, v1, v2, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip2, ip02, ip12));	
		AddTriangleToBuffer(Vector3i(ip12, ip02, ip012));	
	}

	if (r2||r3||TriangleOnStaticBoundary(v0, v2, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip3, ip03, ip023));	
		AddTriangleToBuffer(Vector3i(ip3, ip023, ip2));	
		AddTriangleToBuffer(Vector3i(ip023, ip02, ip2));	
	}	
}



void CMarchingTet::_computeTexCoor2_1(
	 const int v0, const int v1, const int v2, const int v3, CMarchingTetEdge *pSE, CVertexInfo *pVertInfo,
	 Vector3f &t01, Vector3f &t02, Vector3f &t03, Vector3f &t012, Vector3f &t013, Vector3f &t023, Vector3f &t0123)
{
	const int e01=_getEdgePointerByVertexIDs(v0, v1);
	t01=pSE[e01].GetSplitTextCoor(pVertInfo);

	const int e02=_getEdgePointerByVertexIDs(v0, v2);
	t02=pSE[e02].GetSplitTextCoor(pVertInfo);

	const int e03=_getEdgePointerByVertexIDs(v0, v3);
	t03=pSE[e03].GetSplitTextCoor(pVertInfo);

	const int e12=_getEdgePointerByVertexIDs(v1, v2);
	const Vector3f t12=pSE[e12].GetSplitTextCoor(pVertInfo);
	t012 = (t01+t12)*0.5f;

	const int e13=_getEdgePointerByVertexIDs(v1, v3);
	const Vector3f t13=pSE[e13].GetSplitTextCoor(pVertInfo);
	t013 = (t01+t13)*0.5f;

	const Vector3f t0 = pVertInfo[v0].m_vInitPosition;
	const Vector3f t2 = pVertInfo[v2].m_vInitPosition;
	const Vector3f t3 = pVertInfo[v3].m_vInitPosition;
	t023 = (t0+t2+t3)*0.3333333f;
	t0123=(t01+t12+t13)*0.33333f;
}


void CMarchingTet::_computeTexCoor2_2(
	 const int v0, const int v1, const int v2, const int v3, CMarchingTetEdge *pSE, CVertexInfo *pVertInfo,
	 Vector3f &t02, Vector3f &t03, Vector3f &t12, Vector3f &t13, Vector3f &t012, Vector3f &t013, Vector3f &t023, Vector3f &t0123)
{
	const int e01=_getEdgePointerByVertexIDs(v0, v1);
	const Vector3f t01=pSE[e01].GetSplitTextCoor(pVertInfo);

	const int e02=_getEdgePointerByVertexIDs(v0, v2);
	t02=pSE[e02].GetSplitTextCoor(pVertInfo);

	const int e03=_getEdgePointerByVertexIDs(v0, v3);
	t03=pSE[e03].GetSplitTextCoor(pVertInfo);

	const int e12=_getEdgePointerByVertexIDs(v1, v2);
	t12=pSE[e12].GetSplitTextCoor(pVertInfo);

	const int e13=_getEdgePointerByVertexIDs(v1, v3);
	t13=pSE[e13].GetSplitTextCoor(pVertInfo);

	t012 = (t01+t12)*0.5f;
	t013 = (t01+t13)*0.5f;

	const Vector3f t0 = pVertInfo[v0].m_vInitPosition;
	const Vector3f t2 = pVertInfo[v2].m_vInitPosition;
	const Vector3f t3 = pVertInfo[v3].m_vInitPosition;
	t023 = (t0+t2+t3)*0.3333333f;
	t0123=(t01+t12+t13)*0.33333f;
}



void CMarchingTet::_extractSurface2_1(const int tm, 
		const int v0, const int v1, const int v2, const int v3, 
		const Vector3f& P0, const Vector3f& P1, const Vector3f& P2, const Vector3f& P3, 
		CMarchingTetEdge *pSplitEdges, 
		CVertexInfo *pVertInfo)
{
	Vector3f p0=P0, p1=P1, p2=P2, p3=P3;
	Vector3f p01, p02, p03, p12, p13, p012, p013, p023, p0123;

	//setup a local coor. and split
	const bool pflag = pVertInfo[v0].getParticleFlag();
	if (!pflag){//the second vertex is not particle, set a new ref. coor.
		const Vector3i* pBaseTri=_lsearchDynamicVertex(v0, pVertInfo);
		ASSERT0(pBaseTri!=NULL);
		p0 = pVertInfo[v0].m_vCurrPosition;
		p1 = _computeVirtualCoorForV0(v1, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p2 = _computeVirtualCoorForV0(v2, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p3 = _computeVirtualCoorForV0(v3, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
	}

	//perform edge split
	_performSplit(tm, v0, p0, v1, p1, pSplitEdges, pVertInfo);
	_performSplit(tm, v0, p0, v2, p2, pSplitEdges, pVertInfo);
	_performSplit(tm, v0, p0, v3, p3, pSplitEdges, pVertInfo);

	//compute the intermediate vertices;
	_extrapolateVirtualTet2_1(v0, v1, v2, v3, pSplitEdges, pVertInfo, p01, p02, p03, p012, p013, p023, p0123);

	//construct interface and boundary surfaces
	Vector3f t01, t02, t03, t012, t013, t023, t0123;
	_computeTexCoor2_1(v0, v1, v2, v3, pSplitEdges, pVertInfo, t01, t02, t03, t012, t013, t023, t0123);

	addInterfaceSurface2_1(p01, p02, p03, p012, p013, p023, p0123, t01, t02, t03, t012, t013, t023, t0123);
	addBoundarySurface2_1(v0, v1, v2, v3, pVertInfo, 
		pVertInfo[v0].m_vCurrPosition, p01, p02, p03, p012, p013, p023, p0123,
		pVertInfo[v0].m_vInitPosition, t01, t02, t03, t012, t013, t023, t0123);
}


void CMarchingTet::_extractSurface2(const int tm, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo)
{
	int v0, v1, v2, v3, vtmp, eidx01, eidx02, eidx03;
	Vector3f q0, q1, q2, q3, p0, p1, p2, p3, p01, p02, p03, p12, p13, p012, p013, p023, p0123;

	//===========get the vertices and edges for 1st split===========
	_getVerticesAndEdgesByExtractionSequence(0, pVertInfo, v0, v1, v2, v3, eidx01, eidx02, eidx03);
	vtmp = _getVerticeByExtractionSequence(1);	//get the second slit vertex
	reOrderVertices(vtmp, v1, v2, v3, eidx01, eidx02, eidx03); //v1=vtmp now
	//find the base triangle using v2 and v3;
	Vector3i* pBaseTri = _lsearchDynamicEdge(v2, v3, pVertInfo);
	ASSERT0(pBaseTri!=NULL);
	//compute the virtual vertices as if tet is not broken
	p0 = _computeVirtualCoorForV0(v0, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
	p1 = _computeVirtualCoorForV0(v1, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
	p2 = pVertInfo[v2].m_vCurrPosition;
	p3 = pVertInfo[v3].m_vCurrPosition;
	q0=p0, q1=p1, q2=p2, q3=p3;

	const bool pflag = pVertInfo[v0].getParticleFlag();
	if (!pflag){
		const Vector3i* pBaseTri=_lsearchDynamicVertex(v0, pVertInfo);
		ASSERT0(pBaseTri!=NULL);
		p0 = pVertInfo[v0].m_vCurrPosition;
		p1 = _computeVirtualCoorForV0(v1, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p2 = _computeVirtualCoorForV0(v2, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p3 = _computeVirtualCoorForV0(v3, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
	}
	_genPatchForFirstCut(tm, v0, v1, v2, v3, p0, p1, p2, p3, pSplitEdges, pVertInfo);
	p0=q0, p1=q1, p2=q2, p3=q3;

	//===========get the vertices and edges for 2nd split============
	vtmp=v0, v0=v1, v1=vtmp;		//swap v0, v1 to setup a sequence;
	vtmp=v2, v2=v3, v3=vtmp;		//swap v2, v3;
	q0 = p0; p0 = p1; p1 = q0;		//swap p0, p1
	q0 = p2; p2 = p3; p3 = q0;		//swap p2, p3
	_extractSurface2_1(tm, v0, v1, v2, v3, p0, p1, p2, p3, pSplitEdges, pVertInfo);

	//===========get the vertices and edges for 3nd split============
	//this time the two vertices are all on a mesh;
	_performSplit(tm, v2, p2, v0, p0, pSplitEdges, pVertInfo);
	_performSplit(tm, v2, p2, v1, p1, pSplitEdges, pVertInfo);
	_performSplit(tm, v3, p3, v0, p0, pSplitEdges, pVertInfo);
	_performSplit(tm, v3, p3, v1, p1, pSplitEdges, pVertInfo);
	_extrapolateVirtualTet2_2(v0, v1, v2, v3, p0, p1, p2, p3, pSplitEdges,pVertInfo, p02, p03, p12, p13, p012, p013, p023, p0123);

	//extract interface surface and boundary surface
	Vector3f t02, t03, t12, t13, t012, t013, t023, t0123;
	_computeTexCoor2_2(v0, v1, v2, v3, pSplitEdges, pVertInfo, t02, t03, t12, t13, t012, t013, t023, t0123);
	addInterfaceSurface2_2(p02, p03, p12, p13, p012, p013, p023, p0123, t02, t03, t12, t13, t012, t013, t023, t0123);
	addBoundarySurface2_2(v0, v1, v2, v3, pVertInfo, 
		p2, p3, p02, p03, p12, p13, p012, p013, p023, p0123, 
		pVertInfo[v2].m_vInitPosition, pVertInfo[v3].m_vInitPosition, t02, t03, t12, t13, t012, t013, t023, t0123);
}


//======================CASE: CUT THREE===============================
static inline 
Vector3f _vertexChoice(const int v0, const int v1, const Vector3i & tri, CVertexInfo *pVertInfo)
{
	Vector3f p;
	if (v0==v1)
		p = pVertInfo[v0].m_vCurrPosition;
	else
		p = _computeVirtualCoorForV0(v0, tri.x, tri.y, tri.z, pVertInfo);		
	return p;
}

void CMarchingTet::_lockEdges(const int tm, const int v0, const int v1, const int v2, const int v3, CMarchingTetEdge *pSplitEdges)
{
	CMarchingTetEdge *p;
	const int e01=_getEdgePointerByVertexIDs(v0, v1);
	p = &pSplitEdges[e01];
	if (p->AlreadyHasSplit(v0))
		p->Lock(v0, tm);
	const int e02=_getEdgePointerByVertexIDs(v0, v2);
	p = &pSplitEdges[e02];
	if (p->AlreadyHasSplit(v0))
		p->Lock(v0, tm);
	const int e03=_getEdgePointerByVertexIDs(v0, v3);
	p = &pSplitEdges[e03];
	if (p->AlreadyHasSplit(v0))
		p->Lock(v0, tm);
}

void CMarchingTet::_extractSurface3(const int tm, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo)
{
	int v0, v1, v2, v3, vtmp, eidx01, eidx02, eidx03;
	Vector3f q0, p0, p1, p2, p3; 

	//===========get the vertices and edges for 1st split===========
	_getVerticesAndEdgesByExtractionSequence(0, pVertInfo, v0, v1, v2, v3, eidx01, eidx02, eidx03);
	vtmp = _getVerticeByExtractionSequence(1);	//get the second slit vertex
	reOrderVertices(vtmp, v1, v2, v3, eidx01, eidx02, eidx03); //v1=vtmp now
	bool pflag = pVertInfo[v0].getParticleFlag();
	if (pflag){
		//find a base triangle , or not finding
		int nmeshvert, meshvert[4];
		nmeshvert= _getNoneParticleVertices(meshvert, pVertInfo);
		if (nmeshvert==0){ //no vertex is still on a mesh, so no ref point;
			p0 = pVertInfo[v0].m_vCurrPosition;
			p1 = pVertInfo[v1].m_vCurrPosition;
			p2 = pVertInfo[v2].m_vCurrPosition;
			p3 = pVertInfo[v3].m_vCurrPosition;
			_lockEdges(tm, v0, v1, v2, v3, pSplitEdges);
			_lockEdges(tm, v1, v0, v2, v3, pSplitEdges);
			_lockEdges(tm, v2, v0, v1, v3, pSplitEdges);
			_lockEdges(tm, v3, v0, v1, v2, pSplitEdges);
		}
		else{
			Vector3i* pBaseTri = _lsearchDynamicVertex(meshvert[0], pVertInfo);
			ASSERT0(pBaseTri!=NULL);
			p0 = _vertexChoice(v0, meshvert[0], *pBaseTri, pVertInfo);
			p1 = _vertexChoice(v1, meshvert[0], *pBaseTri, pVertInfo);
			p2 = _vertexChoice(v2, meshvert[0], *pBaseTri, pVertInfo);
			p3 = _vertexChoice(v3, meshvert[0], *pBaseTri, pVertInfo);
		}
	}
	else{
		const Vector3i* pBaseTri=_lsearchDynamicVertex(v0, pVertInfo);
		ASSERT0(pBaseTri!=NULL);
		p0 = pVertInfo[v0].m_vCurrPosition;
		p1 = _computeVirtualCoorForV0(v1, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p2 = _computeVirtualCoorForV0(v2, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
		p3 = _computeVirtualCoorForV0(v3, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
	}
	_genPatchForFirstCut(tm, v0, v1, v2, v3, p0, p1, p2, p3, pSplitEdges, pVertInfo);

	//===========get the vertices and edges for 2nd split============
	vtmp=v0, v0=v1, v1=vtmp;		//swap v0, v1 to setup a sequence;
	vtmp=v2, v2=v3, v3=vtmp;		//swap v2, v3;
	q0 = p0; p0 = p1; p1 = q0;		//swap p0, p1
	q0 = p2; p2 = p3; p3 = q0;		//swap p2, p3
	_extractSurface2_1(tm, v0, v1, v2, v3, p0, p1, p2, p3, pSplitEdges, pVertInfo);

	//===========get the vertices and edges for 3nd split============
	_extractSurface2_1(tm, v2, v1, v3, v0, p2, p1, p3, p0, pSplitEdges, pVertInfo);

	//===========get the vertices and edges for 4th split============
	_extractSurface2_1(tm, v3, v1, v0, v2, p3, p1, p0, p2, pSplitEdges, pVertInfo);
}

//=======================================================================
void CMarchingTet::PreSlitAllEdges(const int tm, CMarchingTetEdge* pSplitEdges, CVertexInfo *pVertInfo)
{
	Vector3f p[4] = {pVertInfo[m_tet.x].m_vCurrPosition, 
					 pVertInfo[m_tet.y].m_vCurrPosition, 
					 pVertInfo[m_tet.z].m_vCurrPosition, 
					 pVertInfo[m_tet.w].m_vCurrPosition};
	const int *a = &m_tet.x;
	int i;	
	for (i=0; i<4; i++){
		const int n1 = (i+1)%4;
		const int n2 = (i+2)%4;
		const int n3 = (i+3)%4;
		_performSplit(tm, a[i], p[i], a[n1], p[n1], pSplitEdges, pVertInfo);
		_performSplit(tm, a[i], p[i], a[n2], p[n2], pSplitEdges, pVertInfo);
		_performSplit(tm, a[i], p[i], a[n3], p[n3], pSplitEdges, pVertInfo);
	}

	for (i=0; i<6; i++){
		const int eid = m_pMarchingTetEdge[i];
		CMarchingTetEdge * pedge = &pSplitEdges[eid];
		pedge->m_bParticleFlag[0] = pedge->m_bParticleFlag[0] = true;
		pedge->m_nTime[0] = pedge->m_nTime[1] = tm;
	}
}


void CMarchingTet::ExtractSurface(const int tm, CMarchingTetEdge *pSE, CVertexInfo *pVertInfo)
{
	_ExtractSurface(tm, pSE, pVertInfo);
	return;

	//check split type
	int stype = GetSplitVertexCount();
	switch(stype){
	case 1:
		_extractSurface1(tm, pSE, pVertInfo);
		break;
	case 2:
		_extractSurface2(tm, pSE, pVertInfo);
		break;
	case 3:
	case 4:
		_extractSurface3(tm, pSE, pVertInfo);
		break;
	default:
		assert(0); 
		break;
	}
}


//===================================================================================
inline void CMarchingTet::_getMidEdgePointAttr(const int v0, const int v1, const Vector3f& p0, const Vector3f& p1, CVertexInfo *pVertInfo, int &f, Vector3f &norm)
{
	f = VERTEX_AT_FREE;
	if (_getEdgePointerByVertexIDs(v0, v1)<0){
		f=VERTEX_AT_FIXED;
		return;
	}
	Vector3i *ptri = _lsearchStaticEdge(v0, v1, pVertInfo);
	if (ptri!=NULL || _lsearchDynamicEdge(v0, v1, pVertInfo)!=NULL){
		f = VERTEX_AT_BOUNDARY_LINE;
		norm = p1-p0;
		norm.normalize();
	}
}


inline void CMarchingTet::_getMidTrianglePointAttr(const int v0, const int v1, const int v2, 
	const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, CVertexInfo *pVertInfo, int &f, Vector3f &norm)
{
	f = VERTEX_AT_FREE;
	const Vector3i *ptri = _lsearchDynamicTri(v0, v1, v2, pVertInfo);
	if (ptri!=NULL || _lsearchStaticTri(v0, v1, v2, pVertInfo)!=NULL){
		f = VERTEX_AT_BOUNDARY_PLANE;
		norm = compute_triangle_normal(p0, p1, p2);
	}
}


void CMarchingTet::_decideVertexFlags(const int v0, const int v1, const int v2, const int v3, 
							  		  const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, const Vector3f &p3, 
									  CVertexInfo *pVertInfo, int f[11], Vector3f norms[11])
{
//#define VERTEX_AT_FREE				0
//#define VERTEX_AT_BOUNDARY_LINE		1
//#define VERTEX_AT_BOUNDARY_PLANE		2
//#define VERTEX_AT_FIXED				10
	f[0] = VERTEX_AT_FIXED;		//v0
	f[1] = VERTEX_AT_FIXED;		//v1
	f[2] = VERTEX_AT_FIXED;		//v2
	f[3] = VERTEX_AT_FIXED;		//v3

	_getMidEdgePointAttr(v0, v1, p0, p1, pVertInfo, f[4], norms[4]);
	_getMidEdgePointAttr(v0, v2, p0, p2, pVertInfo, f[5], norms[5]);
	_getMidEdgePointAttr(v0, v3, p0, p3, pVertInfo, f[6], norms[6]);		

	_getMidTrianglePointAttr(v0, v1, v2, p0, p1, p2, pVertInfo, f[7], norms[7]);
	_getMidTrianglePointAttr(v0, v1, v3, p0, p1, p3, pVertInfo, f[8], norms[8]);
	_getMidTrianglePointAttr(v0, v2, v3, p0, p2, p3, pVertInfo, f[9], norms[9]);

	f[10] = VERTEX_AT_FREE;
}


static inline
void addInterfaceSurface4(const int ip01, const int ip02, const int ip03, 
		const int ip012, const int ip013, const int ip023, const int ip0123, const int BID)
{	
	AddTriangleToBuffer(Vector3i(ip01, ip0123, ip012), BID);	
	AddTriangleToBuffer(Vector3i(ip01, ip013, ip0123), BID);	

	AddTriangleToBuffer(Vector3i(ip0123, ip013, ip03), BID);	
	AddTriangleToBuffer(Vector3i(ip0123, ip03, ip023), BID);	

	AddTriangleToBuffer(Vector3i(ip0123, ip02, ip012), BID);	
	AddTriangleToBuffer(Vector3i(ip0123, ip023, ip02), BID);	
}


static inline
void addBoundarySurface4(
		const int v0, const int v1, const int v2, const int v3, CVertexInfo *pVertInfo, 
		const int ip0, const int ip01, const int ip02, const int ip03, 
		const int ip012, const int ip013, const int ip023, const int ip0123, 
		const int BID)
{
	if (TriangleOnStaticBoundary(v0, v1, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip0, ip03, ip013), BID);	
		AddTriangleToBuffer(Vector3i(ip0, ip013, ip01), BID);	
	}

	if (TriangleOnStaticBoundary(v0, v2, v3, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip0, ip02, ip023), BID);	
		AddTriangleToBuffer(Vector3i(ip0, ip023, ip03), BID);	
	}

	if (TriangleOnStaticBoundary(v0, v1, v2, pVertInfo)){
		AddTriangleToBuffer(Vector3i(ip0, ip01, ip012), BID);	
		AddTriangleToBuffer(Vector3i(ip0, ip012, ip02), BID);	
	}
}

static inline 
void ChangeTetVertexSeq(const int vfirst, int &v0, int &v1, int &v2, int &v3)
{
	const int v[4]={v0,v1,v2,v3};
	if (vfirst==v1)
		v0=v[1], v1=v[0], v2=v[3], v3=v[2];
	else if (vfirst==v2)
		v0=v[2], v1=v[0], v2=v[1], v3=v[3];
	else
		v0=v[3], v1=v[0], v2=v[2], v3=v[1];
}


static inline 
void PrepareLocalCoor(const int v0, const int v1, const int v2, const int v3, const Vector3i & tri, CVertexInfo *pVertInfo, Vector3f p[4])
{
	p[0] = pVertInfo[v0].m_vCurrPosition;
	p[1] = _computeVirtualCoorForV0(v1, tri.x, tri.y, tri.z, pVertInfo);		
	p[2] = _computeVirtualCoorForV0(v2, tri.x, tri.y, tri.z, pVertInfo);		
	p[3] = _computeVirtualCoorForV0(v3, tri.x, tri.y, tri.z, pVertInfo);		
}


void CMarchingTet::_extractSurface4(const int tm, 
		const int v0, const int v1, const int v2, const int v3, 
		const Vector3f& P0, const Vector3f& P1, const Vector3f& P2, const Vector3f& P3, const bool useP,
		CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo)
{
	Vector3f p0, p1, p2, p3;
	Vector3f p01, p02, p03, p012, p013, p023, p0123;
	Vector3f t01, t02, t03, t012, t013, t023, t0123;
	Vector3f btri[20];
	int f[20];
	int BID = 0;
	if (pVertInfo[v0].getParticleFlag()) BID=1;

	if (useP){
		p0=P0, p1=P1, p2=P2, p3=P3;
		_performSplit(tm, v0, p0, v1, p1, pSplitEdges, pVertInfo);
		_performSplit(tm, v0, p0, v2, p2, pSplitEdges, pVertInfo);
		_performSplit(tm, v0, p0, v3, p3, pSplitEdges, pVertInfo);
	}
	else{
		//setup a local coor. and split
		const bool pflag = pVertInfo[v0].getParticleFlag();
		if (!pflag){
			//the second vertex is not particle, set a new ref. coor.
			const Vector3i* pBaseTri=_lsearchDynamicVertex(v0, pVertInfo);
			ASSERT0(pBaseTri!=NULL);
			p0 = pVertInfo[v0].m_vCurrPosition;
			p1 = _computeVirtualCoorForV0(v1, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
			p2 = _computeVirtualCoorForV0(v2, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
			p3 = _computeVirtualCoorForV0(v3, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
			_performSplit(tm, v0, p0, v1, p1, pSplitEdges, pVertInfo);
			_performSplit(tm, v0, p0, v2, p2, pSplitEdges, pVertInfo);
			_performSplit(tm, v0, p0, v3, p3, pSplitEdges, pVertInfo);
		}
		else{
			//get the virtual vertices;
			int nmeshvert, meshvert[4];
			nmeshvert= _getNoneParticleVertices(meshvert, pVertInfo);
			if (nmeshvert==0){ //no vertex is still on a mesh, so no ref point;
				p0 = pVertInfo[v0].m_vCurrPosition;
				p1 = pVertInfo[v1].m_vCurrPosition;
				p2 = pVertInfo[v2].m_vCurrPosition;
				p3 = pVertInfo[v3].m_vCurrPosition;
				_lockEdges(tm, v0, v1, v2, v3, pSplitEdges);
				_lockEdges(tm, v1, v0, v2, v3, pSplitEdges);
				_lockEdges(tm, v2, v0, v1, v3, pSplitEdges);
				_lockEdges(tm, v3, v0, v1, v2, pSplitEdges);
			}
			else{
				Vector3i* pBaseTri = _lsearchDynamicVertex(meshvert[0], pVertInfo);
				ASSERT0(pBaseTri!=NULL);
				p0 = _vertexChoice(v0, meshvert[0], *pBaseTri, pVertInfo);
				p1 = _vertexChoice(v1, meshvert[0], *pBaseTri, pVertInfo);
				p2 = _vertexChoice(v2, meshvert[0], *pBaseTri, pVertInfo);
				p3 = _vertexChoice(v3, meshvert[0], *pBaseTri, pVertInfo);
				_performSplit(tm, v0, p0, v1, p1, pSplitEdges, pVertInfo);
				_performSplit(tm, v0, p0, v2, p2, pSplitEdges, pVertInfo);
				_performSplit(tm, v0, p0, v3, p3, pSplitEdges, pVertInfo);
			}
			_extrapolateVirtualTet4(v0, v1, v2, v3, pSplitEdges, pVertInfo, p1, p2, p3);
			p0 = pVertInfo[v0].m_vCurrPosition;
		}
	}

	//compute the intermediate vertices;
	_interpolateTmpVertices4(v0, v1, v2, v3, p0, p1, p2, p3, pSplitEdges, pVertInfo, p01, p02, p03, p012, p013, p023, p0123);
	_computeTexCoor4(v0, v1, v2, v3, pSplitEdges, pVertInfo, t01, t02, t03, t012, t013, t023, t0123);
	_decideVertexFlags(v0, v1, v2, v3, p0, p1, p2, p3, pVertInfo, f, btri);

	//construct the surface;
	const Vector3f t0=pVertInfo[v0].m_vInitPosition;
	const int ip0= AddVertexToBuffer(p0, t0, btri[0], f[0], BID);
	const int ip01= AddVertexToBuffer(p01, t01, btri[4], f[4], BID);
	const int ip02= AddVertexToBuffer(p02, t02, btri[5], f[5], BID);
	const int ip03= AddVertexToBuffer(p03, t03, btri[6], f[6], BID);
	const int ip012= AddVertexToBuffer(p012, t012, btri[7], f[7], BID);
	const int ip013= AddVertexToBuffer(p013, t013, btri[8], f[8], BID);
	const int ip023= AddVertexToBuffer(p023, t023, btri[9], f[9], BID);
	const int ip0123= AddVertexToBuffer(p0123, t0123, btri[10], f[10], BID);

	addInterfaceSurface4(ip01, ip02, ip03, ip012, ip013, ip023, ip0123, BID);
	addBoundarySurface4(v0, v1, v2, v3, pVertInfo, ip0, ip01, ip02, ip03, ip012, ip013, ip023, ip0123, BID);
	//_CheckBuffer(BID);

}

static inline bool _inbuffer(const int v0, const int vbuff[4], const int vlen)
{	
	int i;
	for (i=0; i<vlen; i++) 
		if (v0==vbuff[i]) return true;
	return false;
}
static inline void _setupVertices(
const Vector4i &tet, const Vector3i *pBaseTri, int vbuff[4], const int vlen, CVertexInfo *pVertInfo, Vector3f p[4])
{
	int i;
	const int *a = &tet.x;
	for (i=0; i<4; i++){
		const int v0=a[i];
		if (_inbuffer(v0, vbuff, vlen))
			p[i]=pVertInfo[v0].m_vCurrPosition;
		else
			p[i] = _computeVirtualCoorForV0(v0, pBaseTri->x, pBaseTri->y, pBaseTri->z, pVertInfo);
	}
}
static inline Vector3f _matchVertices(const int v0, const Vector4i &tet, Vector3f vert[4])
{	
	const int *a = &tet.x;
	for (int i=0; i<4; i++){
		if (v0==a[i])
			return vert[i];
	}
	assert(0);
	return Vector3f(0, 0, 0);
}

void CMarchingTet::_ExtractSurface(const int tm, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo)
{
	Vector4i tet;
	Vector3f p0, p1, p2, p3, vert[4];
	int i, v0, v1, v2, v3, vbuff[4];
	const int nsplit = GetSplitVertexCount();
	ASSERT0(nsplit>0);

#ifdef _DEBUG
	tet = m_tet;
	int debn=163;
	if (tet.x==debn || tet.y==debn|| tet.z==debn || tet.w==debn)
		int asgag=1;
	//if (nsplit!=2) return;
#endif

	//for the split vertices;
	for (i=0; i<nsplit; i++){
		if (i==0){
			_getVerticesByExtractionSequence(0, pVertInfo, v0, v1, v2, v3);
			tet = Vector4i(v0, v1, v2, v3);
		}
		else{
			v0=tet.x, v1=tet.y, v2=tet.z, v3=tet.w;
			const int vtmp = _getVerticeByExtractionSequence(i);	//get the second slit vertex
			ChangeTetVertexSeq(vtmp, v0, v1, v2, v3);
		}
		_extractSurface4(tm, v0, v1, v2, v3, p0, p1, p2, p3, 0, pSplitEdges, pVertInfo);
	}
	if (nsplit==4) return;
	
	//for the mesh vertices;
	const int cc= GetNonSplitVertices(vbuff);
	Vector3i tri(vbuff[0], vbuff[1], vbuff[2]), *pBaseTri=NULL;
	switch(cc){
	case 1:
		pBaseTri = _lsearchDynamicVertex(vbuff[0], pVertInfo);
		break;
	case 2:
		pBaseTri = _lsearchDynamicEdge(vbuff[0], vbuff[1], pVertInfo);
		break;
	case 3:
		pBaseTri = &tri;
		break;
	}
	ASSERT0(pBaseTri!=NULL);
	_setupVertices(tet, pBaseTri, vbuff, cc, pVertInfo, vert);

	for (i=0; i<cc; i++){
		v0=tet.x, v1=tet.y, v2=tet.z, v3=tet.w;
		ChangeTetVertexSeq(vbuff[i], v0, v1, v2, v3);
		p0=_matchVertices(v0, tet, vert);
		p1=_matchVertices(v1, tet, vert);
		p2=_matchVertices(v2, tet, vert);
		p3=_matchVertices(v3, tet, vert);
		_extractSurface4(tm, v0, v1, v2, v3, p0, p1, p2, p3, 1, pSplitEdges, pVertInfo);
	}
}


//====================================================================

float CMarchingTetEdge::_computeT(CVertexInfo *pVertInfo)
{
	extern float THdmg;
	float t;
//	return 0.5f;

	const int v0 = m_nP[0];
	const int v1 = m_nP[1];
	const float dmg0 = pVertInfo[v0].J2;
	const float dmg1 = pVertInfo[v1].J2;
	if (dmg0<THdmg){
		if (dmg1>THdmg){
			const float dd = dmg1-dmg0;
			t = (THdmg-dmg0)/dd;
		}
		else{ //forced split;
			t = dmg0/(dmg1+dmg0);
		}
	}
	else{
		if (dmg1<THdmg){
			const float dd = dmg0 - dmg1;
			t = (dmg0-THdmg)/dd;
		}
		else{
			t = dmg0/(dmg1+dmg0);
		}
	}

	t=fabs(t);
	return t;
}

