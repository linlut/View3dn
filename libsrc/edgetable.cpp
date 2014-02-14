//FILE: edgetable.cpp
#include "edgetable.h"

void EdgeTable::buildEdgeTable(void *data)
{
	int i;

	clearEdgeTable();

	if (!m_isSolid){ //For surface patches
		if (m_nVPT==3){
			Vector3i *ptri = (Vector3i*)m_pTri;
			for (i=0; i< m_ntri; i++){
				const Vector3i& tri = ptri[i];
				__buildEdgeTableForOneEdge(i, tri.x, tri.y, data);
				__buildEdgeTableForOneEdge(i, tri.y, tri.z, data);
				__buildEdgeTableForOneEdge(i, tri.z, tri.x, data);
			}
		}
		else if (m_nVPT==4){
			Vector4i *pquad = (Vector4i*)m_pTri;
			for (i=0; i< m_ntri; i++){
				const Vector4i& tri = pquad[i];
				__buildEdgeTableForOneEdge(i, tri.x, tri.y, data);
				__buildEdgeTableForOneEdge(i, tri.y, tri.z, data);
				__buildEdgeTableForOneEdge(i, tri.z, tri.w, data);
				__buildEdgeTableForOneEdge(i, tri.w, tri.x, data);
			}
		}
		else
			assert(0);
		return;
	}


	//for solid element input
	if (m_nVPT==4){
		Vector4i *pquad = (Vector4i*)m_pTri;
		for (i=0; i< m_ntri; i++){
			const Vector4i& tri = pquad[i];
			__buildEdgeTableForOneEdge(i, tri.x, tri.y, data);
			__buildEdgeTableForOneEdge(i, tri.y, tri.z, data);
			__buildEdgeTableForOneEdge(i, tri.z, tri.w, data);
			__buildEdgeTableForOneEdge(i, tri.w, tri.x, data);
			__buildEdgeTableForOneEdge(i, tri.y, tri.w, data);
			__buildEdgeTableForOneEdge(i, tri.x, tri.z, data);
		}
	}
	else if (m_nVPT==8){
		Vector8i *pquad = (Vector8i*)m_pTri;
		for (i=0; i< m_ntri; i++){
			const Vector8i& tri = pquad[i];
			__buildEdgeTableForOneEdge(i, tri.x, tri.y, data);
			__buildEdgeTableForOneEdge(i, tri.y, tri.z, data);
			__buildEdgeTableForOneEdge(i, tri.z, tri.w, data);
			__buildEdgeTableForOneEdge(i, tri.w, tri.x, data);

			__buildEdgeTableForOneEdge(i, tri.x1, tri.y1, data);
			__buildEdgeTableForOneEdge(i, tri.y1, tri.z1, data);
			__buildEdgeTableForOneEdge(i, tri.z1, tri.w1, data);
			__buildEdgeTableForOneEdge(i, tri.w1, tri.x1, data);

			__buildEdgeTableForOneEdge(i, tri.x, tri.x1, data);
			__buildEdgeTableForOneEdge(i, tri.y, tri.y1, data);
			__buildEdgeTableForOneEdge(i, tri.z, tri.z1, data);
			__buildEdgeTableForOneEdge(i, tri.w, tri.w1, data);
		}
	}
	else
		assert(0);
}



inline _VertexEdgeTablePointer *inEdgeList(_VertexEdgeTablePointer * phead, const int v)
{
	_VertexEdgeTablePointer * p = phead;
	while (p!=NULL){
		if (p->m_pEdgeTableElement->m_nV0 == v || p->m_pEdgeTableElement->m_nV1==v)
			return p;
		p=p->m_pNext;
	}
	return NULL;
}
void EdgeTable::__buildEdgeTableForOneEdge(const int triID, const int v0, const int v1, void *userdata)
{
	//if exist in one list, then already processed this edge
	_EdgeTableElementStruct *pedge;
	_VertexEdgeTablePointer *p = inEdgeList(m_ppVETP[v1], v0);
	if (p){
		pedge=p->m_pEdgeTableElement;
		if (pedge->m_nT1==-1)
			pedge->m_nT1 = triID;
		return;
	}

	//if not, insert into both vertices
	const int va = _MIN_(v0, v1);
	const int vb = _MAX_(v0, v1);
	pedge = allocEdgeElement(va, vb, triID, -1, userdata);

	//alloc a pointer and insert into the list
	_VertexEdgeTablePointer *pp = allocEdgePointor();
	pp->m_pEdgeTableElement = pedge;
	pp->m_pNext = m_ppVETP[v0], m_ppVETP[v0]  = pp;

	pp = allocEdgePointor();
	pp->m_pEdgeTableElement = pedge;
	pp->m_pNext = m_ppVETP[v1], m_ppVETP[v1]  = pp;
}
