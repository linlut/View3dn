/*=========================================================
//File: facetable.h
  Nan Zhang, 05/08/2008
  This file is used to build a boundary surface for tetrahedral and 
  quadrilateral meshes. We first build a linked list for each vertex
  on the polygons connected on each vertex, then remove the polygons
  which appear twice there. The left polygons are the boundary polygons

  Histroy:
  05/08/2008: only the triangle boundary is supported yet.
  
=========================================================*/

#ifndef __INC_FACETABLE_ZN05082008_H__
#define __INC_FACETABLE_ZN05082008_H__

#include <sysconf.h>
#include <stdio.h>
#include <vectorall.h>
#include <math.h>
#include <memmgr.h>


class _FaceTableElementStruct
{
public:
	//Store the other vertices in the list
	Vector3i m_vertices;
	int m_counter;
	_FaceTableElementStruct *m_pNext;
	
public:
	_FaceTableElementStruct(void){}

	_FaceTableElementStruct(const int v0, const int v1, const int v2)
	{
		m_vertices = Vector3i(v0, v1, v2);
		m_pNext = NULL;
	}

	void dump(void)
	{
		printf("Face info: v0=%d v1=%d v2=%d\n", m_vertices.x, m_vertices.y, m_vertices.z);
	}
};


class FaceTable: public CMemoryMgr
{

private:
	//number of vertices;
	int m_nv;

	//Pointer for each vertex, a linked list
	_FaceTableElementStruct **m_pFaceTable;

	//allocate one pointer
	_FaceTableElementStruct *_allocFace(const Vector3i &t)
	{
		_FaceTableElementStruct *p = (_FaceTableElementStruct *)Malloc(sizeof(_FaceTableElementStruct));
		p->m_vertices = t;
		p->m_counter = 0;
		return p;
	}

	_FaceTableElementStruct* _insertInfront(_FaceTableElementStruct *head, _FaceTableElementStruct *elm)
	{
		elm->m_pNext = head;
		return elm;
	}

	_FaceTableElementStruct* _faceExist(_FaceTableElementStruct *head, const Vector3i& f)
	{
		_FaceTableElementStruct *p = head;
		while (p!=NULL){
			if (p->m_vertices == f)
				return p;
			p = p->m_pNext;
		}
		return NULL;
	}

	void _updateFaceRecord(const Vector3i& f)
	{
		Vector3i elm(f.y, f.z, 0);	
		_FaceTableElementStruct* p = _faceExist(m_pFaceTable[f.x], elm);
		if (p == NULL){
			_FaceTableElementStruct *newelm = _allocFace(elm);
			m_pFaceTable[f.x] = _insertInfront(m_pFaceTable[f.x], newelm);	
		}
		else{
			p->m_counter++;
		}
	}

	void _collectOneFace(const Vector3i& t0, Vector3i *tribuffer, int &c)
	{
		Vector3i f=t0;
		f.bubbleSort();
		Vector3i elm(f.y, f.z, 0);
		_FaceTableElementStruct* p = _faceExist(m_pFaceTable[f.x], elm);
		ASSERT0(p!=NULL);
		if (p->m_counter==0){
			tribuffer[c++]= t0;
		}
	}

public:

	//clear the edge table
	void clearFaceable(const int nv=0)
	{
		for (int i=0; i<m_nv; i++) m_pFaceTable[i]=NULL;
		Free();
	}

	FaceTable(void): CMemoryMgr()
	{
		m_nv = 0;
		m_pFaceTable = NULL;
	}

	~FaceTable(void)
	{
		Free();
		SafeDeleteArray(m_pFaceTable);
	}
	
	void buildFaceTable(const int nv, const Vector4i *ptet, const int ntet)
	{
		//first, check and init the pointer array
		int i;
		if (m_nv != nv){
			SafeDeleteArray(m_pFaceTable);
			m_pFaceTable = new _FaceTableElementStruct*[nv];
		}
		assert(m_pFaceTable!=NULL);
		m_nv = nv;
		for (i=0; i<nv; i++) m_pFaceTable[i]=NULL;

		//for each tet, get 4 tris and insert to the linked list
		for (i=0; i<ntet; i++){
			Vector4i tet = ptet[i];
			tet.bubbleSort();
			Vector3i tri0(tet.x, tet.y, tet.z);
			_updateFaceRecord(tri0);
			Vector3i tri1(tet.x, tet.y, tet.w);
			_updateFaceRecord(tri1);
			Vector3i tri2(tet.x, tet.z, tet.w);
			_updateFaceRecord(tri2);
			Vector3i tri3(tet.y, tet.z, tet.w);		
			_updateFaceRecord(tri3);
		}	
	}

	//the tet has a specific order 
	Vector3i *collectFaces(const Vector4i *ptet, const int ntet, int & ntri)
	{
		int i;
		//get number of bondary triangles
		int c = 0;
		for (i=0; i<m_nv; i++){
			_FaceTableElementStruct*p =	m_pFaceTable[i];
			while (p!=NULL){
				if (p->m_counter==0) c++;
				p = p->m_pNext;
			}
		}
		ntri = c;
		c = 0;

		//now store them
		Vector3i *ptri = new Vector3i [ntri];
		assert(ptri!=NULL);
		for (i=0; i<ntet; i++){
			Vector4i m = ptet[i];
			Vector3i r0(m.x, m.z, m.y);
			_collectOneFace(r0, ptri, c);
			Vector3i r1(m.x, m.y, m.w);
			_collectOneFace(r1, ptri, c);
			Vector3i r2(m.x, m.w, m.z);
			_collectOneFace(r2, ptri, c);
			Vector3i r3(m.y, m.z, m.w);		
			_collectOneFace(r3, ptri, c);
		}
		return ptri;
	}


};


#endif