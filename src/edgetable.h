//File: edgetable.h

#ifndef __INC_NONEADA_EDGETABLE_H__
#define __INC_NONEADA_EDGETABLE_H__

#include <sysconf.h>
#include <stdio.h>
#include <vectorall.h>
#include <math.h>
#include <memmgr.h>


class _EdgeTableElementStruct
{
public:
	int m_nV0;				//Index of the first vertex
	int m_nV1;				//Index of the second vertex
	int m_nT0;				//Index of the two triangles who share the edge
	int m_nT1;				//if -1, no triangle on that side

	int midp;				//the index of the mid point on the edge
	float e;				//error on the edge
	char m_userdata[8];	//Store customized data

public:
	_EdgeTableElementStruct(){}

	_EdgeTableElementStruct(const int v0, const int v1, const int t0, const int t1)
	{
		m_nV0 = v0, m_nV1 = v1;  
		m_nT0 = t0, m_nT1 = t1; 
	}

	void dump(void)
	{
		printf("Edge info is v0 v1 t0 t1: %d %d %d %d\n", m_nV0, m_nV1, m_nT0, m_nT1);
	}
};


class _VertexEdgeTablePointer
{
public:
	_EdgeTableElementStruct *m_pEdgeTableElement;
	_VertexEdgeTablePointer *m_pNext;

	_VertexEdgeTablePointer(void)
	{
		m_pEdgeTableElement = NULL;
		m_pNext = NULL;
	}

	_VertexEdgeTablePointer(_EdgeTableElementStruct* pelm, _VertexEdgeTablePointer *pnext)
	{
		m_pEdgeTableElement = pelm;
		m_pNext = pnext;
	}
};


class EdgeTable: public CMemoryMgr
{
public:
	//user input connectivity information
	Vector3f *m_pVertex;	//vertices
	int m_nv;				//number of vertices

	int *m_pTri;			//the connectivity buffer
	int m_ntri;				//number of polygons in the connectivity buffer

	int m_nVPT;				//how many verices per triangle or quad

	//internal working buffer
	_VertexEdgeTablePointer **m_ppVETP;	//pointer to a linked list, one entry per vertex

	_EdgeTableElementStruct **m_ppEdge;	//pointer to all the edges allocated
	int m_nEdge;						//edge counter

private:
	//buffer limit
	int m_nMaxEdge;
	int m_nMaxVert;
	int m_isSolid;

private:

	void __buildEdgeTableForOneEdge(const int tid, const int v0, const int v1, void *userdata=NULL);

public:

	//allocate one pointer
	_VertexEdgeTablePointer *allocEdgePointor(void)
	{
		_VertexEdgeTablePointer *p = (_VertexEdgeTablePointer *)Malloc(sizeof(_VertexEdgeTablePointer));
		return p;
	}

	//allocate one element
	virtual _EdgeTableElementStruct *
	allocEdgeElement(const int v0, const int v1, const int t0, const int t1, void* userdata)
	{
		_EdgeTableElementStruct *p = (_EdgeTableElementStruct*)Malloc(sizeof(_EdgeTableElementStruct));
		p->m_nV0 = v0, p->m_nV1 = v1;
		p->m_nT0 = t0, p->m_nT1 = t1;
		m_ppEdge[m_nEdge++] = p;
		ASSERT0(m_nEdge<m_nMaxEdge);
		return p;
	}

	//clear the edge table
	void clearEdgeTable(const int nv=0)
	{
		int n;
		if (nv>0) 
			n=nv;
		else 
			n=m_nv;
		for (int i=0; i<n; i++) m_ppVETP[i]=NULL;
		m_nEdge = 0;
		Free();
	}

	//The good way for adaptively deform a mesh
	void buildEdgeTable(void *userdata=NULL);

	//Extract edges after the edge table has been created
	Vector2i* extractEdges(int &ne)
	{
		Vector2i *p=NULL;
		if (m_nEdge>0){
			ne = m_nEdge;
			p = new Vector2i[m_nEdge];
			for (int i=0; i<m_nEdge; i++){
				const int x = m_ppEdge[i]->m_nV0, 
					      y = m_ppEdge[i]->m_nV1;
				p[i].x = x, p[i].y = y;
			}
		}
		return p;
	}

	//search the edge pointer which contains an edge which starts from v0 to v1
	_VertexEdgeTablePointer * findOneEdgePointer(const int v0, const int v1)
	{
		_VertexEdgeTablePointer * phead = m_ppVETP[v0];
		while (phead!=NULL){
			const _EdgeTableElementStruct* pe = phead->m_pEdgeTableElement;
			if ((pe->m_nV0==v0 && pe->m_nV1==v1)||(pe->m_nV0==v1 && pe->m_nV1==v0))
				return phead;
			phead=phead->m_pNext;
		}
		return phead;
	}

	//search the edge structure which connects two verices v0 and v1
	_EdgeTableElementStruct * findOneEdge(const int v0, const int v1)
	{	
		_EdgeTableElementStruct *pedge=NULL;
		_VertexEdgeTablePointer * p=findOneEdgePointer(v0, v1);
		if (p) 
			pedge = p->m_pEdgeTableElement;
		return pedge;
	}

	//constructor
	EdgeTable(const int isSolid, Vector3f *v, const int nv, int *ptri, const int ntri, const int nvpt, const bool refinemesh=false)
		: CMemoryMgr()
	{
		//support triangle, qua, tetra, and hexahedron
		assert(nvpt==3 || nvpt==4 || nvpt==8);
		if (isSolid!=0)
			m_isSolid = 1;
		else
			m_isSolid = 0;

		m_pVertex = v;
		m_nv = nv;
		m_ntri = ntri;	
		m_pTri = ptri;	
		m_nVPT = nvpt;	

		if (refinemesh){
			m_nMaxEdge = ntri*nvpt*2+2048;
			m_nMaxVert = nv*3+2048;
		}
		else{
			m_nMaxEdge = 2*ntri*nvpt+1024;
			m_nMaxVert = nv;
		}
		m_ppVETP = new _VertexEdgeTablePointer*[m_nMaxVert];
		assert(m_ppVETP!=NULL);
		m_ppEdge = new _EdgeTableElementStruct*[m_nMaxEdge];	
		assert(m_ppEdge!=NULL);
		m_nEdge = 0;						
	}

	//destructor
	~EdgeTable(void)
	{
		Free();
		SafeDeleteArray(m_ppVETP);
		SafeDeleteArray(m_ppEdge);
	}

	//Input: vid is the vertex
	//Return: number of neighbour vertices & neighbour list for vid
	int getNeighbourVerices(const int vid, int neighbours[])
	{
		int count = 0;
		_VertexEdgeTablePointer * head = m_ppVETP[vid];
		while (head!=NULL){
			_EdgeTableElementStruct *p = head->m_pEdgeTableElement;
			ASSERT0(p->m_nV0==vid || p->m_nV1==vid);
			if (p->m_nV0==vid)
				neighbours[count++] = p->m_nV1;
			else
				neighbours[count++] = p->m_nV0;
			head = head->m_pNext;
		}
		return count;
	}

};



class EdgeQueue
{
private:
	//for priority sorting 
	_EdgeTableElementStruct ** m_PriorQueue;
	int m_nPriorQueueCount;

private:

public:

	//priority queue operations
	void clearQueue(void){ m_nPriorQueueCount = 1; }

	bool emptyQueue(void){ return (m_nPriorQueueCount<=1); }

	void enQueue(_EdgeTableElementStruct *p)
	{
		int &n=m_nPriorQueueCount;
		int i = n;
		m_PriorQueue[n++]= p;
		int P=i>>1;
		while ((i!=1) && (m_PriorQueue[i]->e>m_PriorQueue[P]->e)){
			_EdgeTableElementStruct *tmp=m_PriorQueue[i];
			m_PriorQueue[i]=m_PriorQueue[P], m_PriorQueue[P]=tmp;
			i=P, P=i>>1;
		};
	}

	_EdgeTableElementStruct * deQueue(void)
	{
		_EdgeTableElementStruct *retp = m_PriorQueue[1], *tmp;
		int &n=m_nPriorQueueCount;
		m_PriorQueue[1]=m_PriorQueue[--n]; 

		int i=1, L=i<<1, R=L+1, c=0;
		while (L<n){
			if ((c++)>1000) 
				printf("Deadloop!\n");
			if (R<n){
			   if ((m_PriorQueue[i]->e<m_PriorQueue[L]->e) || (m_PriorQueue[i]->e<m_PriorQueue[R]->e)){
				   int K=R;
				   if (m_PriorQueue[L]->e>m_PriorQueue[R]->e) K=L;
				   tmp=m_PriorQueue[i], m_PriorQueue[i]=m_PriorQueue[K], m_PriorQueue[K]=tmp;
				   i=K, L=i<<1, R=L+1;
			   }
			   else
				   break;
			}
			else{
				if (m_PriorQueue[i]->e<m_PriorQueue[L]->e)
				   tmp=m_PriorQueue[i], m_PriorQueue[i]=m_PriorQueue[L], m_PriorQueue[L]=tmp;
				break; 
			}
		}
		return retp;
	}

	EdgeQueue(const int nsize=4096)
	{
		int s = nsize;
		if (s<4096) s=4096;
		m_PriorQueue = new _EdgeTableElementStruct*[s];
		assert(m_PriorQueue!=NULL);
		clearQueue();
	}

	~EdgeQueue(void)
	{
		SafeDeleteArray(m_PriorQueue);
	}

};

#endif