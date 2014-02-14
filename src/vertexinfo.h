//Vertexinfo.h

#ifndef __INC_VERTEXINFO_H__
#define __INC_VERTEXINFO_H__

#include <sysconf.h>
#include <memmgr.h>
#include <dynamic_array.h>
#include <vectorall.h>
#include <object3d.h>

#include "dlinkedlist.h"


class CTriangleListItem
{
public:
	int m_v0;
	int m_v1;
	CTriangleListItem * m_pNext;

public:
	CTriangleListItem(void)
	{
		m_pNext = NULL;
	}

	~CTriangleListItem(){}
};


class CQuadListItem
{
public:
	int m_v0;
	int m_v1;
	int m_v2;
	CQuadListItem * m_pNext;

public:
	CQuadListItem(void)
	{
		m_pNext = NULL;
	}

	~CQuadListItem(){}
};


class CVertexInfo: public CDLinkedListItem
{
private:
	unsigned short m_bFlags;

public:

	unsigned short m_nMaterialID;	//material;

	float J2;
	
	Vector3f m_vInitPosition;		//initial position;

	Vector3f m_vCurrPosition;		//current position;
	
	CTriangleListItem * m_pStaticTriList;	//

	CTriangleListItem * m_pDynamicTriList;	//

public:
	CVertexInfo(void): CDLinkedListItem()
	{
		J2 = 0;
		m_bFlags = 0;
		m_nMaterialID = 0xffff;
		m_pStaticTriList = m_pDynamicTriList = NULL;
	}

	~CVertexInfo(){}

	//==================================
	void setParticleFlag(bool t)
	{
		m_bFlags &= 0xfffe;
		if (t) m_bFlags |= 0x1;
	}

	void setHasFlatCutFlag(bool t)
	{
		m_bFlags &= 0xfffd;
		if (t) m_bFlags |= 0x2;
	}

	void setVisitFlag(bool t)
	{
		m_bFlags &= 0x7fff;
		if (t) m_bFlags |= 0x8000;
	}

	void setMaterialID(const int matid)
	{
		m_nMaterialID = (unsigned short)matid;
	}

	//==================================
	bool getStaticBoundaryFlag(void) const
	{
		if (m_pStaticTriList) return true;
		return false;
	}

	bool getParticleFlag(void) const
	{
		const int r = m_bFlags & 0x1;
		return (r == 0x01);
	}

	bool getHasFlatCutFlag(void)
	{
		const int r = m_bFlags & 0x2;
		return (r == 0x02);
	}

	bool getDynBoundaryFlag(void) const
	{
		if (m_pDynamicTriList) return true;
		return false;
	}

	bool getVisitFlag(void) const
	{
		const int r = (m_bFlags & 0x8000);
		return (r == 0x8000);
	}

	int getMaterialID(void) const
	{
		return (int)m_nMaterialID;
	}
};


inline 
Vector3i* __lsearchVertexOnBoundarySurface(const int v0, const CTriangleListItem *p)
{
	static Vector3i tri;
	if (p){
		tri.x=v0, tri.y=p->m_v0, tri.z=p->m_v1;
		return &tri;
	}
	return NULL;
}


inline
Vector3i* _lsearchStaticVertex(const int v0, CVertexInfo *pVertex)
{
	CTriangleListItem *p = pVertex[v0].m_pStaticTriList;
	return __lsearchVertexOnBoundarySurface(v0, p);
}


inline
Vector3i* _lsearchDynamicVertex(const int v0, CVertexInfo *pVertex)
{
	CTriangleListItem *p = pVertex[v0].m_pDynamicTriList;
	return __lsearchVertexOnBoundarySurface(v0, p);
}


inline 
Vector3i* __lsearchEdgeOnBoundarySurface(const int v0, const int v1, CTriangleListItem *pHead)
{
	static Vector3i tri;
	CTriangleListItem *p = pHead;
	while (p!=NULL){
		if (p->m_v0 == v0){
			tri.x = v0, tri.y = p->m_v1, tri.z = v1;
			return &tri;
		}
		if (p->m_v1 == v0){
			tri.x = v0, tri.y = v1, tri.z = p->m_v0;
			return &tri;
		}
		p=p->m_pNext;
	}
	return NULL;
}


inline
Vector3i* _lsearchStaticEdge(const int v0, const int v1, CVertexInfo *pVertex)
{
	CTriangleListItem *p = pVertex[v1].m_pStaticTriList;
	return __lsearchEdgeOnBoundarySurface(v0, v1, p);
}


inline
Vector3i* _lsearchDynamicEdge(const int v0, const int v1, CVertexInfo *pVertex)
{
	CTriangleListItem *p = pVertex[v1].m_pDynamicTriList;
	return __lsearchEdgeOnBoundarySurface(v0, v1, p);
}


inline 
Vector3i* __lsearchTriOnBoundarySurface(const int v0, const int v1, const int v2, const CTriangleListItem *pHead)
{
	static Vector3i tri;
	const CTriangleListItem *p = pHead;
	while (p!=NULL){
		if (p->m_v0 == v0 && p->m_v1 == v1){
			tri.x = v0, tri.y=v1, tri.z=v2;
			return &tri;
		}
		if (p->m_v0 == v1 && p->m_v1 == v0){
			tri.x = v0, tri.y=v2, tri.z=v1;
			return &tri;
		}
		p=p->m_pNext;
	}
	return NULL;
}


inline 
Vector3i* _lsearchStaticTri(const int v0, const int v1, const int v2, CVertexInfo *pVertex)
{
	const CTriangleListItem *p = pVertex[v2].m_pStaticTriList;
	return __lsearchTriOnBoundarySurface(v0, v1, v2, p);
}


inline 
Vector3i* _lsearchDynamicTri(const int v0, const int v1, const int v2, CVertexInfo *pVertex)
{
	const CTriangleListItem *p = pVertex[v2].m_pDynamicTriList;
	return __lsearchTriOnBoundarySurface(v0, v1, v2, p);
}


inline
bool __VerticesConnectedOnBoundarySurface(const int v0, const int v1, const CTriangleListItem *pHead)
{
	bool r = false;
	const CTriangleListItem *p = pHead;
	while (p!=NULL){
		if (p->m_v0==v1 || p->m_v1==v1){
			r = true;
			break;
		}
		p = p->m_pNext;
	}
	return r;
}


inline
bool VerticesConnectedOnStaticBoundary(const int v0, const int v1, const CVertexInfo *pVertex)
{
	const CTriangleListItem *p = pVertex[v0].m_pStaticTriList;
	return __VerticesConnectedOnBoundarySurface(v0, v1, p);
}


inline
bool VerticesConnectedOnDynamicBoundary(const int v0, const int v1, const CVertexInfo *pVertex)
{
	const CTriangleListItem *p = pVertex[v0].m_pDynamicTriList;
	return __VerticesConnectedOnBoundarySurface(v0, v1, p);
}


inline
bool TriangleOnStaticBoundary(const int v0, const int v1, const int v2, CVertexInfo *pVertInfo)
{
	//return true;
	if (!pVertInfo[v0].getStaticBoundaryFlag())
		return false;
	if (!pVertInfo[v1].getStaticBoundaryFlag())
		return false;
	if (!pVertInfo[v2].getStaticBoundaryFlag())
		return false;
	if (_lsearchStaticTri(v0, v1, v2, pVertInfo))
		return true;
	return false;
}

inline CTriangleListItem* 
HasOpsiteSideTriangle(const Vector3i & tri, const int maxv, CTriangleListItem *pHead)
{
	const int v0 = maxv;
	int v1, v2;
	if (v0==tri.x)
		v1=tri.y, v2=tri.z;
	else if (v0==tri.y)
		v1=tri.z, v2=tri.x;
	else
		v1=tri.x, v2=tri.y;

	CTriangleListItem *p = pHead;
	while (p!=NULL){
		if (p->m_v0==v2 && p->m_v1==v1)
			return p;
		if (p->m_v0==v1 && p->m_v1==v2)
			return p;
		p=p->m_pNext;
	}
	return NULL;
}



void BuildStaticTriangleTabForVertices(CVertexInfo *pVertex, const int nv, const Vector3i *ptri, const int ntri, CMemoryMgr &mgr);

void BuildDynamicTriangleTabForVertices(CVertexInfo *pVertex, const int nv, const Vector3i *ptri, const int ntri, CMemoryMgr &mgr);

int CheckConnectedVerticesOnDynBoundary(const Vector4i & tet, const CVertexInfo *pVertInfo, int v[4][4], int vlen[4]);

int ClassifyVerticesOfTet(const Vector4i & t, const CVertexInfo *pVertInfo, int v[4][4], int vlen[4]);

void SetFirstVertexWithMaxEdgeIntersections(const Vector4i &tet, int v[4], const int vlen, const CVertexInfo *pVertInfo);

void ComputeStaticMeshBoundingBox(const int matid, const CVertexInfo *pVertInfo, const int nv, AxisAlignedBox& box);


#endif