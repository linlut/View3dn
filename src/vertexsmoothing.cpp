
#include <dynamic_array.h>
#include "vertexsmoothing.h"
#include "vertexinfo.h"
#include "geomath.h"


class CQsortCell
{
public:
	CGeneratedVertex elm;
	int m_nID;

public: 
	void Assign(const CGeneratedVertex& _elm, int id)
	{
		elm=_elm;
		m_nID = id;
	}
};


static inline 
int dictionary_compare(const Vector3f &v1, const Vector3f &v2, const float errorbound)
{
	if (fabs(v1.x - v2.x)< errorbound){
		if (fabs(v1.y - v2.y)< errorbound){
			if (fabs(v1.z - v2.z)< errorbound){
				return 0;
			}
			else{
				if (v1.z < v2.z)
					return -1;
				else 
					return 1;
			}
		}
		else{
			if (v1.y<v2.y)
				return -1;
			else
				return 1;
		}
	}
	else{
		if (v1.x < v2.x) 
			return -1;
		else
			return 1;
	}
	return 1;
}


static double SAMEPOINT_TOLERANCE;
static int compare(const void *e1, const void *e2)
{
	const CQsortCell *p1 = (CQsortCell *)e1;
	const CQsortCell *p2 = (CQsortCell *)e2;
	int r =  dictionary_compare(p1->elm.v, p2->elm.v, SAMEPOINT_TOLERANCE);
	return r;
}

static inline 
int _isValidTriangle(CGeneratedVertex* pvert, const int v0, const int v1, const int v2)
{
	Vector3f d1, d2, d3;
	double dot;
	const Vector3f *p0 = &pvert[v0].v;
	const Vector3f *p1 = &pvert[v1].v;
	const Vector3f *p2 = &pvert[v2].v;
	d1 = *p1 - *p0;
	d2 = *p2 - *p1;
	d3 = CrossProd(d1, d2);
	dot = DotProd(d3,d3);
	if (fabs(dot)<1e-30){
		//fprintf(stderr, "Vertex 0: %d (%f,%f,%f)\n", v0, (float)p0->x, (float)p0->y, (float)p0->z);
		//fprintf(stderr, "Vertex 1: %d (%f,%f,%f)\n", v1, (float)p1->x, (float)p1->y, (float)p1->z);
		//fprintf(stderr, "Vertex 2: %d (%f,%f,%f)\n", v2, (float)p2->x, (float)p2->y, (float)p2->z);
		return 0;
	}
	return 1;
}

static inline
int ValidateTriangles(Vector3i *ptri, const int ntri, CGeneratedVertex *pvert)
{
	int i, c;
	bool *pflag = new bool[ntri]; assert(pflag!=NULL);
	for (i=0; i<ntri; i++) pflag[i]=true;

	for (i=0; i<ntri; i++){
		const int v0 = ptri[i].x; 
		const int v1 = ptri[i].y; 
		const int v2 = ptri[i].z;
		if ((v0==v1) || (v0==v2) || (v1==v2) || (!_isValidTriangle(pvert,v0,v1,v2)))
			pflag[i]=false;
	}
	for (i=c=0; i<ntri; i++){
		if (pflag[i])
			ptri[c++]=ptri[i];
	}
	delete []pflag;
	return c;
}


void VertexClusterGVertices(CDynamicArray<CGeneratedVertex>& fractVertex, CDynamicArray<Vector3i> &fractTri, const float TH)
{
	int i, count;
	CQsortCell *pBuffer;
	int *pIDBuffer;

	SAMEPOINT_TOLERANCE =TH;
	Vector3i *ptri = fractTri.GetBuffer();
	CGeneratedVertex *pvert = fractVertex.GetBuffer();
	int ntri = fractTri.GetSize();
	int nv = fractVertex.GetSize();

	pBuffer = new CQsortCell[nv];
	pIDBuffer = new int[nv];
	assert(pBuffer!=NULL);
	assert(pIDBuffer!=NULL);
	for (i=0; i<nv; i++) pBuffer[i].Assign(pvert[i], i);
	qsort(pBuffer, nv, sizeof(CQsortCell), compare);

	//Assign new vertices to the array;
	pIDBuffer[pBuffer[0].m_nID] = count = 0;
	pvert[0] = pBuffer[0].elm;
	for (i=1; i<nv; i++){
		CQsortCell *p0 = &pBuffer[i-1];
		CQsortCell *p1 = &pBuffer[i];
		if (compare(p0, p1)){
			count++;
			pvert[count] = pBuffer[i].elm;
		}
		else{
			int attr = _MAX_(p0->elm.attr, p1->elm.attr);
			pvert[count].attr = _MAX_(attr,pvert[count].attr);
		}
		pIDBuffer[pBuffer[i].m_nID] = count;
	}
	nv = count+1;
	fractVertex.Resize(nv);

	//Reassign face table indexs;
	for (i=0; i<ntri; i++){
		Vector3i *p = &ptri[i];
		const int v0=p->x;
		const int v1=p->y;
		const int v2=p->z;
		p->x = pIDBuffer[v0];
		p->y = pIDBuffer[v1];
		p->z = pIDBuffer[v2];
		if (p->x<0 || p->y<0 || p->z<0 || p->x>100000){
			int aga=1;
		}
	}	
	delete [] pBuffer;
	delete [] pIDBuffer;

	ntri=ValidateTriangles(ptri, ntri, pvert);
	fractTri.Resize(ntri);
}


//============================SMOOTHING=====================================


static inline CTriangleListItem** 
BuildDynamicTriangleTabForGVertices(const int nv, const Vector3i *ptri, const int ntri, CMemoryMgr &mgr)
{
	int i, j;
	CTriangleListItem **plist=new CTriangleListItem*[nv];
	assert(plist!=NULL);
	for (i=0; i<nv; i++) plist[i]=NULL;

	for (i=0; i<ntri; i++){
		const int* a = &ptri[i].x;
		for (j=0; j<3; j++){
			const int v0 = a[j];
			const int v1 = a[(j+1)%3];
			const int v2 = a[(j+2)%3];
			CTriangleListItem * pListItem = (CTriangleListItem*)mgr.Malloc(sizeof(CTriangleListItem));
			pListItem->m_v0 = v1;
			pListItem->m_v1 = v2;
			pListItem->m_pNext = plist[v0];
			plist[v0] = pListItem;
		}
	}
	return plist;
}



static inline void _getAverageVec(CGeneratedVertex* pvert, 
	const CTriangleListItem* phead, Vector3f &sump, Vector3f &sumt)
{
	int c=0;
	sump=sumt=Vector3f(0, 0, 0);
	const CTriangleListItem* p=phead;
	while (p!=NULL){
		const int v0 = p->m_v0;
		const int v1 = p->m_v1;
		sump+=pvert[v0].v;
		sump+=pvert[v1].v;
		sumt+=pvert[v0].t;
		sumt+=pvert[v1].t;
		p=p->m_pNext;
		c+=2;
	}
	const float div = 1.0f/c;
	sump *= div;
	sumt *= div;
}


static inline void _getAverageVecBoundary(const int node, const int btype, CGeneratedVertex* pvert, 
	const CTriangleListItem* phead, CVertexInfo *pVertInfo, Vector3f &sump, Vector3f &sumt)
{
	int c=0;
	sump=sumt=Vector3f(0, 0, 0);
	const CTriangleListItem* p=phead;

	while (p!=NULL){
		const int v0 = p->m_v0;
		const int v1 = p->m_v1;
		if (pvert[v0].attr!=VERTEX_AT_FREE){
			sump+=pvert[v0].v;
			sumt+=pvert[v0].t;
			c++;
		}
		if (pvert[v1].attr!=VERTEX_AT_FREE){
			sump+=pvert[v1].v;
			sumt+=pvert[v1].t;
			c++;
		}
		p=p->m_pNext;
	}
	if (c==0)
		sump = pvert[node].v, sumt = pvert[node].t;
	else{
		const float div = 1.0f/c;
		sump *= div;
		sumt *= div;
		//project to the limitation plane or surf.
		const Vector3f& p0 = pvert[node].v;
		const Vector3f& n = pvert[node].norm;
		if (btype==VERTEX_AT_BOUNDARY_LINE){ //limit is a line;
			const Vector3f dif= sump-p0;
			const float dist = DotProd(dif, n);
			sump = p0+dist*n; 
		}
		else{//limit should be a plane;
			const float d = -DotProd(p0, n);
			const float dist = DotProd(sump, n) + d;
			sump -= n*dist;
		}

	}
}

static float K = 0.10f;
static void _smoothingVertices(CGeneratedVertex* pvert, const int nv, CTriangleListItem** plist, CVertexInfo *pVertInfo)
{
	int i;
	Vector3f ap, at;
	Vector3f *pnewvert = new Vector3f[nv];
	Vector3f *pnewtex = new Vector3f[nv];
	assert(pnewvert!=NULL && pnewtex!=NULL);

	for (i=0; i<nv; i++){
		if (plist[i]==NULL){
			pnewvert[i] = pvert[i].v; 
			pnewtex[i] = pvert[i].t; 
			continue;
		}
		const int flag = pvert[i].attr;
		switch(flag){
		case VERTEX_AT_FIXED:
			ap = pvert[i].v; at = pvert[i].t;
			break;
		case VERTEX_AT_FREE:
			_getAverageVec(pvert, plist[i], ap, at);
			break;
		case VERTEX_AT_BOUNDARY_LINE:
		case VERTEX_AT_BOUNDARY_PLANE:
			_getAverageVecBoundary(i, flag, pvert, plist[i], pVertInfo, ap, at);
			break;
		default:
			assert(0);
		}
		Vector3f dp = ap -  pvert[i].v;
		Vector3f dt = at -  pvert[i].t;
		pnewvert[i] = pvert[i].v + K * dp; 
		pnewtex[i] = pvert[i].t + K * dt; 
	}
	for (i=0; i<nv; i++){
		pvert[i].v=pnewvert[i];
		pvert[i].t=pnewtex[i];
	}
	delete [] pnewvert;
	delete [] pnewtex;
}



void SmoothingVertices(CGeneratedVertex* pvert, const int nv, const Vector3i *ptri, const int ntri, CMemoryMgr &mgr, CVertexInfo *pVertInfo, const float SCALE)
{
	if (nv<=0 || ntri<=0) return;
	if (SCALE<1E-6) return;
	CTriangleListItem** plist;	
	plist=BuildDynamicTriangleTabForGVertices(nv, ptri, ntri, mgr);
	assert(plist!=NULL);
	K = SCALE;

	//do smoothing...
	_smoothingVertices(pvert, nv, plist, pVertInfo);
	_smoothingVertices(pvert, nv, plist, pVertInfo);
	_smoothingVertices(pvert, nv, plist, pVertInfo);

	delete []plist;
}