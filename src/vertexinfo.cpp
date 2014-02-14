//Vertexinfo.cpp

#include "vertexinfo.h"
#include "object3d.h"

void BuildStaticTriangleTabForVertices(CVertexInfo *pVertex, const int nv, const Vector3i *ptri, const int ntri, CMemoryMgr &mgr)
{
	int i, j;

	for (i=0; i<nv; i++) pVertex[i].m_pStaticTriList=NULL;
	for (i=0; i<ntri; i++){
		const int* a = &ptri[i].x;
		for (j=0; j<3; j++){
			const int v0 = a[j];
			const int v1 = a[(j+1)%3];
			const int v2 = a[(j+2)%3];
			CTriangleListItem * pListItem = (CTriangleListItem*)mgr.Malloc(sizeof(CTriangleListItem));
			pListItem->m_v0 = v1;
			pListItem->m_v1 = v2;
			pListItem->m_pNext = pVertex[v0].m_pStaticTriList;
			pVertex[v0].m_pStaticTriList = pListItem;
		}
	}
}


void BuildDynamicTriangleTabForVertices(CVertexInfo *pVertex, const int nv, const Vector3i *ptri, const int ntri, CMemoryMgr &mgr)
{
	int i, j;

	for (i=0; i<nv; i++) pVertex[i].m_pDynamicTriList=NULL;
	for (i=0; i<ntri; i++){
		const int* a = &ptri[i].x;
		for (j=0; j<3; j++){
			const int v0 = a[j];
			const int v1 = a[(j+1)%3];
			const int v2 = a[(j+2)%3];
			CTriangleListItem * pListItem = (CTriangleListItem*)mgr.Malloc(sizeof(CTriangleListItem));
			pListItem->m_v0 = v1;
			pListItem->m_v1 = v2;
			pListItem->m_pNext = pVertex[v0].m_pDynamicTriList;
			pVertex[v0].m_pDynamicTriList = pListItem;
		}
	}
}


static inline int INDEX_OF_VECTOR4I(const int v, const Vector4i tet)
{
	const int *a = &tet.x;
	for (int i=0; i<4; i++)
		if (v==a[i]) return i;
	return -1;
}


int CheckConnectedVerticesOnDynBoundary(const Vector4i & tet, const CVertexInfo *pVertInfo, int v[4][4], int vlen[4])
{
	CDLinkedListItem node[4];
	int i, j, c=0;
	const int *a = &tet.x;
	
	//do clustering;
	for (i=0; i<4; i++) node[i].m_nIndex=i;
	for (i=0; i<4; i++){
		const int vv = a[i];
		CDLinkedListItem * pi = &node[i];
		CTriangleListItem* p= pVertInfo[vv].m_pDynamicTriList;	
		while (p!=NULL){
			j = INDEX_OF_VECTOR4I(p->m_v0, tet);
			if (j!=-1 && vv<a[j])
				pi->mergeList(&node[j]);
			j = INDEX_OF_VECTOR4I(p->m_v1, tet);
			if (j!=-1 && vv<a[j])
				pi->mergeList(&node[j]);
			p = p->m_pNext;
		}	
	}

	//check the # of sets and output
	bool visited[4]={false, false, false, false};
	for (i=0; i<4; i++){
		if (visited[i]) continue;
		const CDLinkedListItem *ptr=&node[i];
		visited[i]=true;
		v[c][0] = a[i], 
		vlen[c]=1;;
		int & k = vlen[c];
		CDLinkedListItem * phead = ptr->m_pNext;
		while (phead!=ptr){
			const int idx = phead->m_nIndex;
			visited[idx]=true;
			v[c][k++] = a[idx];
			phead = phead->m_pNext;
		}
		c++;
	}

	return c;
}



int ClassifyVerticesOfTet(const Vector4i & t, const CVertexInfo *pVertInfo, int v[4][4], int vlen[4])
{
	int n[4] = {pVertInfo[t.x].m_nIndex,
				pVertInfo[t.y].m_nIndex,
				pVertInfo[t.z].m_nIndex,
				pVertInfo[t.w].m_nIndex};
	bool visited[4]={false, false, false, false};
	const int *a = &t.x;
	int i, j, nset = 0;

	for (i=0; i<4; i++){
		int & c = vlen[nset]; c=0;
		if (visited[i]) continue;
		v[nset][c] = a[i], c++;
		visited[i]=true;
		for (j=i+1; j<4; j++){
			if (n[i]==n[j]){
				v[nset][c]=a[j], c++;
				visited[j]=true;
			}
		}
		nset++;
	}
	return nset;
}



static inline
bool EdgeIntersectIsosurface(const int v0, const int v1, const CVertexInfo *pVertInfo)
{
	extern float THdmg;
	const float f0 = pVertInfo[v0].J2;
	const float f1 = pVertInfo[v1].J2;
	if ((f0<THdmg && f1>THdmg) || (f0>THdmg && f1<THdmg))
		return true;
	return false;
}

void SetFirstVertexWithMaxEdgeIntersections(const Vector4i &tet, int v[4], const int vlen, const CVertexInfo *pVertInfo)
{
	const int *a = &tet.x;
	int i, c[4]={0,0,0,0};

	for (i=0; i<vlen; i++){
		const int v0 = v[i];
		int idx = INDEX_OF_VECTOR4I(v0, tet);
		const int v1 = a[(idx+1)%4];
		const int v2 = a[(idx+2)%4];
		const int v3 = a[(idx+3)%4];
		if (EdgeIntersectIsosurface(v0, v1, pVertInfo)) 
			c[i]++;
		if (EdgeIntersectIsosurface(v0, v2, pVertInfo)) 
			c[i]++;
		if (EdgeIntersectIsosurface(v0, v3, pVertInfo)) 
			c[i]++;
	}

	int cmax = c[0], ptr=0;
	for (i=1; i<vlen; i++){
		if (c[i]>cmax){
			cmax = c[i];
			ptr = i;
		}
	}

	//finally, swap the max and the first;
	int tmp = v[0];
	v[0] = v[ptr];
	v[ptr]=tmp;
}


void ComputeStaticMeshBoundingBox(const int matid, const CVertexInfo *pVertInfo, const int nv, AxisAlignedBox& box)
{
	box.minp = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
	box.maxp = Vector3d(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
	for (int i=0; i<nv; i++){
		const CVertexInfo *p = &pVertInfo[i];
		if (p->getMaterialID()!=matid) continue;
        Vector3d pos(p->m_vInitPosition.x, p->m_vInitPosition.y, p->m_vInitPosition.z);
		Minimize(box.minp, pos);
		Maximize(box.maxp, pos);
	}
}


