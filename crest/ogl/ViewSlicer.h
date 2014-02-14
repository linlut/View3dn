//FILE: ViewSlicer.h
#ifndef _VIEW_SLICER_H_ZN20080220
#define _VIEW_SLICER_H_ZN20080220

#include <sysconf.h>
#include <vectorall.h>
#include <vector>
#include <zlinkedlist.h>
#include <geomath.h>

#define VIEWSLICER_BUCKET_LENGTH 12000

namespace cia3d{
namespace ogl{

class ViewSlicer
{
public: 
	class ElementInfo
	{
	public:
		int depth;	
		int elementID;
	};

	typedef LinkedListNode<ElementInfo> ListNode;

private:
	Vector3f m_viewpos;		//view position
	Vector3f m_viewdir;		//view direction
	float m_dh;				//slice gap

	float* m_pDistArray;	//Array of distance for all the vertices
	int m_nDistArray;		//number of vertices

	unsigned int m_nCounter;//counter for running times
	int m_nSlice;
	int m_nCurrentSlice;
	ListNode * m_listHead[VIEWSLICER_BUCKET_LENGTH];
	ListNode * m_pActiveList;

	//==================================
	ListNode * m_pListNodeBuffer;
	int m_nBufferCount;

private:

	//return pre node for linkage
	ListNode* _scanCollectListTet(const ListNode* root, const Vector4i *pelm, Vector4i *pout, int &nout)
	{
		ListNode* pre=NULL;
		const ListNode* p = root;
		while (p!=NULL){
			pout[nout++] = pelm[p->data.elementID];
			pre = (ListNode*)p;
			p = p->pnext;
		}
		return pre;
	}

	ListNode* _scanCollectListHex(const ListNode* root, const Vector8i *phex, Vector4i *pout, int &nout)
	{
		const ListNode* p = root;
		ListNode* pre=NULL;
		while (p!=NULL){
			hexConnectivityToTetConnectivity(phex[p->data.elementID], &pout[nout]);
			nout+=6;
			pre = (ListNode*)p;
			p = p->pnext;
		}
		return pre;
	}

	//Merge two lists
	ListNode* _concatTwoLists(ListNode* active, ListNode* root, const int *pelm, const int elmtype, int *pout, int &nout)
	{
		const ListNode* rr = active;
		nout = 0;

		//if at least one list is empty
		if (active==NULL || root==NULL){ 
			if (active==NULL) rr = root;
			if (elmtype==4)
				_scanCollectListTet(rr, (const Vector4i*)pelm, (Vector4i*)pout, nout);
			else
				_scanCollectListHex(rr, (const Vector8i*)pelm, (Vector4i*)pout, nout);
		}
		else{
			ListNode* tail;
			if (elmtype==4){
				tail=_scanCollectListTet(active, (const Vector4i*)pelm, (Vector4i*)pout, nout);
				_scanCollectListTet(root, (const Vector4i*)pelm, (Vector4i*)pout, nout);
			}
			else{
				tail=_scanCollectListHex(active, (const Vector8i*)pelm, (Vector4i*)pout, nout);
				_scanCollectListHex(root, (const Vector8i*)pelm, (Vector4i*)pout, nout);
			}
			ASSERT0(tail!=NULL);
			tail->pnext = (ListNode*)root;
		}
		return (ListNode*)rr;
	}

	ListNode *_deleteNodes(ListNode *head, const int scanlineid)
	{
		ListNode *newhead = head;
		ListNode *p=head;
		ListNode *pre=NULL;
		while(p!=NULL){
			if (p->data.depth==scanlineid){ //To be deleted
				if (pre)
					pre->pnext = p->pnext;
				if (newhead==p)
					newhead = p->pnext;
			}
			else{
				pre = p;
			}
			p=p->pnext;
		}
		return newhead;
	}

	//return: head of the list
	ListNode* _scanCollectListTet2(const int scanlineID, ListNode* root, const Vector4i *pelm, Vector4i *pout, int &nout, ListNode *&newtail)
	{
		ListNode *newhead = NULL, *p = root;
		newtail = NULL;
		while (p!=NULL){
			ListNode *bakptr = p->pnext;
			pout[nout++] = pelm[p->data.elementID];
			if (p->data.depth != scanlineID){
				newhead = InsertAheadLinkedListNode(newhead, p);
				if (!newtail) newtail = p;
			}
			p = bakptr;
		}
		return newhead;
	}

	ListNode* _scanCollectListHex2(const int scanlineID, ListNode* root, const Vector8i *phex, Vector4i *pout, int &nout, ListNode *&newtail)
	{
		ListNode *newhead = NULL, *p = root;
		newtail = NULL;
		while (p!=NULL){
			ListNode *bakptr = p->pnext;
			hexConnectivityToTetConnectivity(phex[p->data.elementID], &pout[nout]);
			nout+=6;
			if (p->data.depth != scanlineID){
				newhead = InsertAheadLinkedListNode(newhead, p);
				if (!newtail) newtail = p;
			}
			p = bakptr;
		}
		return newhead;
	}

	//Merge two lists
	ListNode* _concatTwoLists2(const int scanlineID, ListNode* active, ListNode* root, const int *pelm, const int elmtype, int *pout, int &nout)
	{
		ListNode* rr = active;
		nout = 0;
		//if at least one list is empty
		if (active==NULL || root==NULL){ 
			if (active==NULL) rr = root;
			ListNode *newtail;
			if (elmtype==4)
				rr = _scanCollectListTet2(scanlineID, rr, (const Vector4i*)pelm, (Vector4i*)pout, nout, newtail);
			else
				rr = _scanCollectListHex2(scanlineID, rr, (const Vector8i*)pelm, (Vector4i*)pout, nout, newtail);
		}
		else{
			ListNode *newhead1, *newhead2, *newtail1, *newtail2;
			if (elmtype==4){
				newhead1 = _scanCollectListTet2(scanlineID, active, (const Vector4i*)pelm, (Vector4i*)pout, nout, newtail1);
				newhead2 = _scanCollectListTet2(scanlineID, root, (const Vector4i*)pelm, (Vector4i*)pout, nout, newtail2);
			}
			else{
				newhead1 = _scanCollectListHex2(scanlineID, active, (const Vector8i*)pelm, (Vector4i*)pout, nout, newtail1);
				newhead2 = _scanCollectListHex2(scanlineID, root, (const Vector8i*)pelm, (Vector4i*)pout, nout, newtail2);
			}
			if (newhead1==NULL)
				rr = newhead2;
			else{
				ASSERT0(newtail1!=NULL);
				newtail1->pnext = newhead2;
				rr = newhead1;
			}
		}
		return rr;
	}

	ListNode *Malloc(void)
	{
		ListNode *p = &m_pListNodeBuffer[m_nBufferCount];
		m_nBufferCount++;
		return p;
	}

	void _computeDistance2View(const void *vert, const int floatsize, const int nv, float &distMin, float &distMax);

	void _bucketSortElements(const int *pelm, const int nelm, const int elmtype, const int direction, const float DMIN, const float DMAX);

public:
	void setSliceDistance(const float dh)
	{
		m_dh = dh;
	}

	float getSliceDistance(void)
	{
		return m_dh;
	}

	void setCamera(const Vector3f &viewpos, const Vector3f& dir)
	{
		m_viewpos = viewpos;
		m_viewdir = Normalize(dir);
	}

	ViewSlicer(void)
	{
		Vector3f viewpos(0,0,0);
		Vector3f dir(-1,0,0);
		const float dh=1;
		setCamera(viewpos, dir);
		setSliceDistance(dh);

		m_pDistArray = NULL;
		m_nDistArray = 0;
		m_nCounter = 0;

		m_pListNodeBuffer=NULL;
		m_nBufferCount = 0;
	}

	~ViewSlicer(void)
	{
		SafeDeleteArray(m_pDistArray);
		SafeDeleteArray(m_pListNodeBuffer);
		m_nBufferCount = 0;
	}

	float bucketSort(const void*vert, const int floatsize, const int nv, const int *pelm, const int nelm, const int elmtype, const int direction);

	bool bucketOneStep(const int *pelm, const int elmtype, int *pout, int &nout)
	{
		bool r;
		if (m_nCurrentSlice>=m_nSlice){
			nout = 0;
			r = false;
		}
		else{
			m_pActiveList = _concatTwoLists2(m_nCurrentSlice, m_pActiveList, m_listHead[m_nCurrentSlice], pelm, elmtype, pout, nout);
			m_nCurrentSlice++;
			r = true;
		}
		return r;
	}

};

}//ogl
}//cia3d


#endif