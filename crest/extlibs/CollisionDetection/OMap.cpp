/////////////////////////////////////////////////////////////////////////////
/// Collision Detection
/// CREST 2008
/////////////////////////////////////////////////////////////////////////////
//#include "StdAfx.h"
//#include "Share.h"
#include "OMap.h"
#include "math.h"
#include "MathDefs.h"
//#include "ExternDeclare.h"
//#include "PerformMonitor.h"
#include "Tritri.h"
//#include "Parti.h"
//#include "OcclusionTest.h"
//#include "ColliResp.h"
//#include "MI.h"
#include "CrestObjects.h"

// Added for mem debuger
#define CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC 

#define CR_II 0
#define CR_IT 1
#define CR_TT 1

#include <stdlib.h>
#include <crtdbg.h>

#define HASH_PRIME  1019 //7//31//53//61//101//503//991//997//1013//1019//1021//1117//4001//5987//8009//9973//20011//
#define PAIR_PRIME  1019
#define PAIR_INDEX_MODE 1 // stapling 1; 2 big 2; parti 3

#define DETECTION_LEVEL 2
/////////////////////////////////////////////////////////////////////////////
COMap::COMap()
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); // Memory buffer debuger
	m_nCountTCell=0;
	m_dwR=10;//20;//10;//9;//8;//5;// subdivision resolution in mm
	//m_OMap0.get_allocator().allocate(2048);
	m_nAPairs=m_nTPairs=m_nPPairs=0;	//collision pairs
}

COMap::~COMap()
{
	//DeleteOMap();
	//if(!m_OMap.empty()) TRACE("m_OMap is not empty\n");
}

inline void COMap::DeleteOMap()
{
	//TRACE("%d\n", m_OMap.size());
	//for(DWORD i=0; i<HASH_PRIME; i++) m_OMap[i].clear();
}

//int COMap::LookUpMap()
//{
//	//static LARGE_INTEGER Counter; 	//float millisec;
//	//e_performMoniter.RecordTime(&Counter, TRUE, 'O');
//	//e_performMoniter.MarkTime(&Counter);
//
//	//if(e_mi.m_iGrab>0 && e_iHold>0) return e_mi.m_iGrab;
//	//DeleteOMap();
//	tOMap m_OMap[HASH_PRIME];
//	
//	//m_OMap.reserve(1019);
//	//long l=m_OMap.capacity();
//	//m_OMap.get_allocator().allocate(2048);
//	
//	tTissueLink* pTissue=e_share.m_pTissueList;
//	tInstrumentLink* pInstrument=e_share.m_pInstrumentList;
//	CGeoModel* pG;
//	CDeformable* pD;
//	vertex* pV, *pT;
//	long nLengthV, nLengthT;
//	
//	/////////////////////////////////////////////////////////////////////////
//	//tCounterMap counterMap;	//tCounterMap counterMap[HASH_PRIME];
//
//	//while(pInstrument)
//	//{
//	//	pG=&(pInstrument->geoModel);
//	//	pV=pG->GetVertex(&nLengthV);
//	//	pT=pG->GetTriangle(&nLengthT);
//	//	FillCounters(counterMap, pInstrument->id, pV, nLengthV, pT, nLengthT);
//	//	pInstrument=pInstrument->pNext;
//	//}
//	//while(pTissue)
//	//{
//	//	pD=&(pTissue->deformable);
//	//	if(e_mi.m_iGrab>0) pD->GeoFollowCPos();//if(e_iHold==1 && e_mi.m_iGrab>0) // Important for grasping
//	//	pG=&(pTissue->geoModel);
//	//	pV=pG->GetVertex(&nLengthV);
//	//	pT=pG->GetTriangle(&nLengthT);
//	//	FillCounters(counterMap, pTissue->id, pV, nLengthV, pT, nLengthT);
//	//	pTissue=pTissue->pNext;
//	//}
//
//	//if(e_share.m_partiSys.m_emitter.particleCount) // change to partiLink later
//	//	FillCountersParti(counterMap, e_share.m_partiSys);
//
//	//tCounterMap::iterator it=counterMap.begin();
//	//DWORD dwCell;
//	//while(it!=counterMap.end())
//	//{
//	//	dwCell=(*it).first;
//	//	if((*it).second.first>1)
//	//		m_OMap[Hash(dwCell)][dwCell];
//	//	it++;
//	//}
//	//	//DWORD dwCell;
//	//	//for(long c=0; c<HASH_PRIME*6; c++)
//	//	//{
//	//		//tCounterMap& cmap = counterMap[c];
//	//		//tCounterMap::iterator it=cmap.begin();
//	//		//while(it!=cmap.end())
//	//		//{
//	//		//	dwCell=(*it).first;
//	//		//	if((*it).second.first>1) m_OMap[Hash(dwCell)][dwCell];
//	//		//	it++;
//	//		//}
//	//	//}
//	////e_performMoniter.RecordTime(&Counter, FALSE, 'O');
//	////TRACE("counterMap size is %d\n", counterMap.size());
//	////TRACE("OMap size is %d\n", m_OMap.size());
//	/////////////////////////////////////////////////////////////////////////
//	//e_performMoniter.MarkTime(&Counter);
//
//	pTissue=e_share.m_pTissueList;
//	pInstrument=e_share.m_pInstrumentList;
//	while(pInstrument)
//	{
//		pG=&(pInstrument->geoModel);
//		pV=pG->GetVertex(&nLengthV);
//		pT=pG->GetTriangle(&nLengthT);
//		FillOMapT(m_OMap, pInstrument->id, pV, nLengthV, pT, nLengthT);
//		pInstrument=pInstrument->pNext;
//	}
//	while(pTissue)
//	{
//		pD=&(pTissue->deformable);
//		if(e_mi.m_iGrab>0) pD->GeoFollowCPos();//if(e_iHold==1 && e_mi.m_iGrab>0) // Important for grasping
//		pG=&(pTissue->geoModel);
//		pV=pG->GetVertex(&nLengthV);
//		pT=pG->GetTriangle(&nLengthT);
//		FillOMapT(m_OMap, pTissue->id, pV, nLengthV, pT, nLengthT);
//		pTissue=pTissue->pNext;
//	}
//	//FillOMapParti(m_OMap);
//
//	if(e_share.m_partiSys.m_emitter.particleCount>1) // change to partiLink later
//		FillOMapParti(m_OMap, e_share.m_partiSys);
//		//FillOMapParti(m_OMap);
//
//	int nFlag=-1;
//	nFlag = DetectCRNode(m_OMap);
//	//nFlag = DetectCRParti(m_OMap);
//	
//	//e_performMoniter.RecordTime(&Counter, FALSE, 'O');
//	
//	//MeasureCP(m_OMap);
//	//MeasureCPParti(m_OMap);
//
//	return nFlag;
//}

int COMap::LookUpMap(CrestObjects *pObjects)
{
	int nFlag=-1;
	//static LARGE_INTEGER Counter; 	//float millisec;
	//e_performMoniter.RecordTime(&Counter, TRUE, 'O');
	//e_performMoniter.MarkTime(&Counter);

	//if(e_mi.m_iGrab>0 && e_iHold>0) return e_mi.m_iGrab;
	//DeleteOMap();
	tOMap m_OMap[HASH_PRIME];
	
	//m_OMap.reserve(1019);
	//long l=m_OMap.capacity();
	//m_OMap.get_allocator().allocate(2048);
	
	//tTissueLink* pTissue=e_share.m_pTissueList;
	//tInstrumentLink* pInstrument=e_share.m_pInstrumentList;
	//CGeoModel* pG;
	//CDeformable* pD;
	//vertex* pV, *pT;
	float *pV;
	long *pT;
	long nLengthV, nLengthT;	//int nObjectType;
	long nObjects=pObjects->m_nObjects;
	CrestObjectInfo* pObjInfo=pObjects->m_pObjectInfo;
	
	/////////////////////////////////////////////////////////////////////////
	//tCounterMap counterMap;	//tCounterMap counterMap[HASH_PRIME];

	//while(pInstrument)
	//{
	//	pG=&(pInstrument->geoModel);
	//	pV=pG->GetVertex(&nLengthV);
	//	pT=pG->GetTriangle(&nLengthT);
	//	FillCounters(counterMap, pInstrument->id, pV, nLengthV, pT, nLengthT);
	//	pInstrument=pInstrument->pNext;
	//}
	//while(pTissue)
	//{
	//	pD=&(pTissue->deformable);
	//	if(e_mi.m_iGrab>0) pD->GeoFollowCPos();//if(e_iHold==1 && e_mi.m_iGrab>0) // Important for grasping
	//	pG=&(pTissue->geoModel);
	//	pV=pG->GetVertex(&nLengthV);
	//	pT=pG->GetTriangle(&nLengthT);
	//	FillCounters(counterMap, pTissue->id, pV, nLengthV, pT, nLengthT);
	//	pTissue=pTissue->pNext;
	//}

	//if(e_share.m_partiSys.m_emitter.particleCount) // change to partiLink later
	//	FillCountersParti(counterMap, e_share.m_partiSys);

	//tCounterMap::iterator it=counterMap.begin();
	//DWORD dwCell;
	//while(it!=counterMap.end())
	//{
	//	dwCell=(*it).first;
	//	if((*it).second.first>1)
	//		m_OMap[Hash(dwCell)][dwCell];
	//	it++;
	//}
	//	//DWORD dwCell;
	//	//for(long c=0; c<HASH_PRIME*6; c++)
	//	//{
	//		//tCounterMap& cmap = counterMap[c];
	//		//tCounterMap::iterator it=cmap.begin();
	//		//while(it!=cmap.end())
	//		//{
	//		//	dwCell=(*it).first;
	//		//	if((*it).second.first>1) m_OMap[Hash(dwCell)][dwCell];
	//		//	it++;
	//		//}
	//	//}
	////e_performMoniter.RecordTime(&Counter, FALSE, 'O');
	////TRACE("counterMap size is %d\n", counterMap.size());
	////TRACE("OMap size is %d\n", m_OMap.size());
	/////////////////////////////////////////////////////////////////////////
	//e_performMoniter.MarkTime(&Counter);

	for(long i=0;i<nObjects;i++)
	{
		pV=(pObjInfo+i)->pVertex;
		pT=(pObjInfo+i)->pPolygon;
		nLengthV=(pObjInfo+i)->nVertex;
		nLengthT=(pObjInfo+i)->nPolygon;
		//if(e_mi.m_iGrab>0) pD->GeoFollowCPos();//if(e_iHold==1 && e_mi.m_iGrab>0) // Important for grasping
		FillOMapT(m_OMap, (pObjInfo+i)->nID, pV, nLengthV, pT, nLengthT);
	}
	//FillOMapParti(m_OMap);

	//if(e_share.m_partiSys.m_emitter.particleCount>1) // change to partiLink later
	//	FillOMapParti(m_OMap, e_share.m_partiSys);
	//	//FillOMapParti(m_OMap);

	nFlag = MarchingCells(m_OMap, pObjects);
	//nFlag = DetectCRParti(m_OMap);
	
	//e_performMoniter.RecordTime(&Counter, FALSE, 'O');
	
	//MeasureCP(m_OMap);
	//MeasureCPParti(m_OMap);

	return nFlag;
}

//BOOL COMap::IsNewCRNode(long n, bool bRNode, CDeformable* pD)
//{
//	long nTotal;
//	tCRNode* pCRNode;
//
//	if(bRNode)
//	{
//		pCRNode=pD->m_response_b.RForce;
//		nTotal=pD->m_response_b.nRForce;
//		for(long j=0; j<nTotal; j++)
//		{
//			if(n==pCRNode[j].id) return FALSE;
//		}
//	}
//	else{
//		for(long i=0; i<INSTRUMENT_MAX; i++)
//		{
//			pCRNode=pD->m_response_b.CPos[i];
//			nTotal=pD->m_response_b.nCPos[i];
//			for(long j=0; j<nTotal; j++)
//			{
//				if(n==pCRNode[j].id) return FALSE;
//			}
//		}
//	}
//
//	return TRUE;
//}

///////////////////////////////////////////////////////////////////////////////
//// Construct object counting table
//void COMap::FillCounters(tCounterMap& counterMap, long nObject, vertex* v, long nV, vertex* t, long nT)
//{
//	long idT; 
//	long i;
//	vertex vp[3]; // vertices of triangle primitive
//	DWORD dwCell;
//
//	for(idT=0;idT<nT;idT++)
//	{
//		*(vp)=*(v+(t+idT)->p1);
//		*(vp+1)=*(v+(t+idT)->p2);
//		*(vp+2)=*(v+(t+idT)->p3);
//		RasterAABB(vp, 3, m_idTCell, m_nCountTCell, RASTER_SIZE);
//
//		//tPrimCell &pc=m_opc[nObject];
//		//tCidVector &cid=pc[idT];
//		//cid.clear();
//		//for(i=0;i<m_nCountTCell;i++)
//		//{
//		//	cid.push_back(*(m_idTCell+i));
//		//}
//
//		for(i=0;i<m_nCountTCell;i++)
//		{
//			dwCell=*(m_idTCell+i);
//			tCo* pCo=&counterMap[dwCell]; //tCo* pCo=&counterMap[Hash(dwCell)][dwCell];
//			if(pCo->first<1){ // new
//				pCo->first=1;
//				pCo->second=nObject;
//			}
//			else if(pCo->second!=nObject)
//			{
//				pCo->first++;
//				pCo->second=nObject;
//			}
//		}
//	}
//}

/////////////////////////////////////////////////////////////////////////////
// Fill OHC with objects 
//void COMap::FillOMapT(tOMap* m_OMap, long nObject, vertex* v, long nV, vertex* t, long nT)
//{
//	long idT; 
//	long i;
//	vertex vp[3]; // vertices of triangle primitive
//
//	for(idT=0;idT<nT;idT++)
//	{
//		*(vp)=*(v+(t+idT)->p1);
//		*(vp+1)=*(v+(t+idT)->p2);
//		*(vp+2)=*(v+(t+idT)->p3);
//		RasterAABB(vp, 3, m_idTCell, m_nCountTCell, RASTER_SIZE);
//		
//		for(i=0;i<m_nCountTCell;i++)
//		{
//			FillCell(m_OMap, nObject, idT, *(m_idTCell+i));
//		}
//	}
//}

void COMap::FillOMapT(tOMap* m_OMap, long nObject, CDFloat* v, long nV, long* t, long nT)
{
	long idT; 
	vertex vp[3]; // vertices of triangle primitive
	int iT, i;

	for(idT=0;idT<nT;idT++)
	{
		for(i=0;i<3;i++)
		{
			iT=*(t+idT*3+i);
			(vp+i)->x=*(v+iT);
			(vp+i)->y=*(v+iT+1);
			(vp+i)->z=*(v+iT+2);
		}
		RasterAABB(vp, 3, m_idTCell, m_nCountTCell, RASTER_SIZE);
		
		for(i=0;i<m_nCountTCell;i++)
		{
			FillCell(m_OMap, nObject, idT, *(m_idTCell+i));
		}
	}
}

inline void COMap::FillCell(tOMap* m_OMap, long nObject, long nVid, DWORD dwCell)
{

		//tObjMap &obj=m_OMap[dwCell];
		//tVidList &vList=obj[nObject];
		//vList.push_front(nVid);
		
		//((m_OMap[dwCell])[nObject]).push_back(nVid);
		//tOMap::iterator it=m_OMap.find(dwCell);
		//if(it!=m_OMap.end())
		//{
		//	((*it).second)[nObject].push_back(nVid);
		//}

		//m_OMap[Hash(dwCell)][dwCell];//[nObject]).push_back(nVid);
		//tOMap::iterator it=m_OMap[Hash(dwCell)].find(dwCell);
		//if(it!=m_OMap.end())
		//{
		//	((*it).second)[nObject].push_back(nVid);
		//}

	m_OMap[Hash(dwCell)][dwCell][nObject].push_back(nVid); // without countermap
	//tOMap& omap = m_OMap[Hash(dwCell)];// with countermap...
	//tOMap::iterator it=omap.find(dwCell);
	//if(it!=omap.end())
	//{
	//	((*it).second)[nObject].push_back(nVid);
	//}

		//	tOMap::iterator p;
		//	if( m_OMap.end() == (p=m_OMap.find(dwCell)) ) // New cell
		//	{
		//	}
		//	else{ // Add to existing cell
		//		(*p).second;
		//	}
	
}

/////////////////////////////////////////////////////////////////////////////
// Rasterizaiton of an primtive in AABB
// v=primtive vertices buffer address, nV=vertices count
// nCidLimit=the max length of cell id buffer
// pCid=cell id buffer address
// nCid=number of rasterized cell id
inline void COMap::RasterAABB(vertex* v, long nV, DWORD* pCid, long& nCid, const long nCidLimit)
{
	long i,j,k;
	DWORD dwCell;
	
	vertex vmin, vmax; //AABB
	long xmin, ymin, zmin, xmax, ymax, zmax; // Rasterized AABB

	vmin.x=vmax.x=(*v).x;
	vmin.y=vmax.y=(*v).y;
	vmin.z=vmax.z=(*v).z;
	for(i=1;i<nV;i++)
	{
		vmin.x=__min(vmin.x, (*(v+i)).x);
		vmax.x=__max(vmax.x, (*(v+i)).x);
		vmin.y=__min(vmin.y, (*(v+i)).y);
		vmax.y=__max(vmax.y, (*(v+i)).y);
		vmin.z=__min(vmin.z, (*(v+i)).z);
		vmax.z=__max(vmax.z, (*(v+i)).z);
	}
	xmin=Real2Int(vmin.x)/m_dwR;
	xmax=Real2Int(vmax.x)/m_dwR;
	ymin=Real2Int(vmin.y)/m_dwR;
	ymax=Real2Int(vmax.y)/m_dwR;
	zmin=Real2Int(vmin.z)/m_dwR;
	zmax=Real2Int(vmax.z)/m_dwR;

	nCid=0;
	for(k=zmin;k<=zmax;k++)
		for(j=ymin;j<=ymax;j++)
			for(i=xmin;i<=xmax;i++)
			{
				if(nCid>=nCidLimit)
				{
					//TRACE("Cell id RASTER_SIZE overflows.\n");
				}
				else{
					dwCell=i+50+(j+50)*100+(k+50)*10000; // x, y, z [-500,500], Related to dwR.
					*(pCid+nCid++)=dwCell;
				}
			}
}

//int COMap::DetectCRNode(tOMap* m_OMap)
//{// Vector versioin
//
//	//static LARGE_INTEGER Counter; 	//float millisec;
//	//e_performMoniter.RecordTime(&Counter, TRUE, 'om');
//	//e_performMoniter.MarkTime(&Counter);
//
//	//tCollisionSet cset[PAIR_PRIME];
//	////tCollisionSet cset;
//	//m_nAPairs=m_nTPairs=m_nPPairs=0;
//	//m_nPPairs=m_nDPairs=0;
//	m_nPPairs=0;
//
//	//long i,j;
//
//	long nId[2]; // Objects info
//	long nType[2];
//	CGeoModel* pG[2];
//	CDeformable* pD[2];
//
//
//	vertex* pV[2];// Primitives info
//	vertex* pT[2];
//	long nLengthV, nLengthT;
// 	
//	vertex its1, its2; // Intersection segment
//	int nCoplanar=0;
//
//	int nGrab=-1;
//
//	vertex &v1=m_tp[0].pos[0]; // The 6 vertices of two triangles
//	vertex &v2=m_tp[0].pos[1]; 
//	vertex &v3=m_tp[0].pos[2]; 
//	vertex &v4=m_tp[1].pos[0]; 
//	vertex &v5=m_tp[1].pos[1]; 
//	vertex &v6=m_tp[1].pos[2]; 
//
//	//e_mi.m_iGrab=-1; // Release existing control nodes
//	// Clear C R nodes
//	tTissueLink* pTissue=e_share.m_pTissueList;
//	while(pTissue)
//	{	
//		//pTissue->deformable.ClearCRNodes();
//		pTissue->deformable.ClearCRNodes(FALSE, TRUE);
//		pTissue=pTissue->pNext;
//	}
//
//	for(long c=0; c<HASH_PRIME; c++)
//	{
//	tOMap& omap = m_OMap[c];
//	tOMap::iterator p=omap.begin();// p to cell
//	while(p!=omap.end())
//	{
//		tObjMap &obj=(*p).second; // objmap in one cell 
//		if(obj.size()>=2)
//		{
//			tObjMap::iterator pObj0=obj.begin(); // pObj to obj id/vlist
//			while(pObj0!=obj.end()) // pObj0 to obj
//			{
//				nId[0]=(*pObj0).first;
//				nType[0]=e_share.GetModel(nId[0], pG, pD);
//				pV[0]=pG[0]->GetVertex(&nLengthV);
//				pT[0]=pG[0]->GetTriangle(&nLengthT);
//				tObjMap::iterator pObj1=pObj0; pObj1++; // reset pObj1 to the next obj
//				while(pObj1!=obj.end()) // obj1 goes over
//				{
//					nId[1]=(*pObj1).first;
//					nType[1]=e_share.GetModel(nId[1], pG+1, pD+1);
//					pV[1]=pG[1]->GetVertex(&nLengthV);
//					pT[1]=pG[1]->GetTriangle(&nLengthT);
//					tVidVector &vVector0=(*pObj0).second;
//					tVidVector::iterator pVector0=vVector0.begin(); // reset pList0 to v. of obj0
//					while(pVector0!=vVector0.end())
//					{
//						v1=(pV[0])[m_tp[0].id[0]=(pT[0])[*pVector0].p1]; // check intersection:
//						v2=(pV[0])[m_tp[0].id[1]=(pT[0])[*pVector0].p2];
//						v3=(pV[0])[m_tp[0].id[2]=(pT[0])[*pVector0].p3];
//						tVidVector &vVector1=(*pObj1).second;
//						tVidVector::iterator pVector1=vVector1.begin(); // reset pList1 to v. of obj1
//						while(pVector1!=vVector1.end()) // collision response
//						{
//							//m_nAPairs++;
//							// EIT cellwise filter start ---
//							//if(!CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
//							//{
//							//	pVector1++;
//							//	continue;
//							//}
//							// EIT cellwise filter over ---
//							v4=(pV[1])[m_tp[1].id[0]=(pT[1])[*pVector1].p1]; //check intersection:
//							v5=(pV[1])[m_tp[1].id[1]=(pT[1])[*pVector1].p2];
//							v6=(pV[1])[m_tp[1].id[2]=(pT[1])[*pVector1].p3];
//							//m_nTPairs++;
//							//e_performMoniter.MarkTime(&Counter);
//							if( !tri_tri_intersection_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//																(float*)&v4, (float*)&v5, (float*)&v6, 
//																&nCoplanar,
//																(float*)&its1, (float*)&its2) 
//																&& 1!=nCoplanar)
//							//if( !tri_tri_overlap_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							//if( !NoDivTriTriIsect((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							{
//								//e_performMoniter.RecordTime(&Counter, FALSE, 'i');
//								pVector1++;
//								continue;
//							}
//							m_nPPairs++;
//							// PIT cellwise filter start ---
//							//if(CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
//							// PIT cellwise filter over ---
//							{
//								//m_nDPairs++;
//								nGrab=1;		
//								CallResponse(nId, nType, pD);
//							}
//							pVector1++;
//						}
//						pVector0++;
//					}
//					pObj1++;			
//				}
//				pObj0++;
//			}
//		}
//		p++;
//	}
//	}
//
//	// Swap CR buffers. Disable it when timing CD
//	pTissue=e_share.m_pTissueList;
//	while(pTissue)
//	{	
//		//pTissue->deformable.ClearCRNodes();
//		pTissue->deformable.CopyResponse();
//		pTissue=pTissue->pNext;
//	}
//
//	e_share.GraspingCheck();
//
//	return e_mi.m_iGrab=nGrab;
//}

int COMap::MarchingCells(tOMap* m_OMap, CrestObjects *pObjects)
{// Vector versioin

	//static LARGE_INTEGER Counter; 	//float millisec;
	//e_performMoniter.RecordTime(&Counter, TRUE, 'om');
	//e_performMoniter.MarkTime(&Counter);

	//tCollisionSet cset[PAIR_PRIME];
	////tCollisionSet cset;
	//m_nAPairs=m_nTPairs=m_nPPairs=0;
	//m_nPPairs=m_nDPairs=0;
	m_nPPairs=0;

	long i,j;

	long nId[2]; // Objects info
	long nType[2];
	//CGeoModel* pG[2];
	//CDeformable* pD[2];
	CrestObjectInfo *pObjectInfo[2];


	//vertex* pV[2];// Primitives info
	//vertex* pT[2];
	//long nLengthV, nLengthT;
 	CDFloat* pV[2];
	long* pT[2];
	CDFloat* pCurrentV;
	long* pCurrentT;
	
	vertex its1, its2; // Intersection segment
	int nCoplanar=0;

	int nGrab=-1;

	vertex &v1=m_tp[0].pos[0]; // The 6 vertices of two triangles
	vertex &v2=m_tp[0].pos[1]; 
	vertex &v3=m_tp[0].pos[2]; 
	vertex &v4=m_tp[1].pos[0]; 
	vertex &v5=m_tp[1].pos[1]; 
	vertex &v6=m_tp[1].pos[2]; 

	////e_mi.m_iGrab=-1; // Release existing control nodes
	//// Clear C R nodes
	//tTissueLink* pTissue=e_share.m_pTissueList;
	//while(pTissue)
	//{	
	//	//pTissue->deformable.ClearCRNodes();
	//	pTissue->deformable.ClearCRNodes(FALSE, TRUE);
	//	pTissue=pTissue->pNext;
	//}
	pObjects->ClearCRNodes(false,true);

	for(long c=0; c<HASH_PRIME; c++)
	{
	tOMap& omap = m_OMap[c];
	tOMap::iterator p=omap.begin();// p to cell
	while(p!=omap.end())
	{
		tObjMap &obj=(*p).second; // objmap in one cell 
		if(obj.size()>=2)
		{
			tObjMap::iterator pObj0=obj.begin(); // pObj to obj id/vlist
			while(pObj0!=obj.end()) // pObj0 to obj
			{
				nId[0]=(*pObj0).first;
				pObjectInfo[0]=pObjects->GetObjectInfo(nId[0]);
				nType[0]=pObjectInfo[0]->nObjectType;
				pV[0]=pObjectInfo[0]->pVertex;
				pT[0]=pObjectInfo[0]->pPolygon;
				tObjMap::iterator pObj1=pObj0; pObj1++; // reset pObj1 to the next obj
				while(pObj1!=obj.end()) // obj1 goes over
				{
					nId[1]=(*pObj1).first;
					pObjectInfo[1]=pObjects->GetObjectInfo(nId[1]);
					nType[1]=pObjectInfo[1]->nObjectType;
					pV[1]=pObjectInfo[1]->pVertex;
					pT[1]=pObjectInfo[1]->pPolygon;
					tVidVector &vVector0=(*pObj0).second;
					tVidVector::iterator pVector0=vVector0.begin(); // reset pList0 to v. of obj0
					while(pVector0!=vVector0.end())
					{
						//v1=(pV[0])[m_tp[0].id[0]=(pT[0])[*pVector0].p1]; // check intersection:
						//v2=(pV[0])[m_tp[0].id[1]=(pT[0])[*pVector0].p2];
						//v3=(pV[0])[m_tp[0].id[2]=(pT[0])[*pVector0].p3];
						pCurrentT=*pT+(*pVector0)*3;
						pCurrentV=*pV+(*pCurrentT++)*3;
						v1.x=*(pCurrentV++); v1.y=*(pCurrentV++); v1.z=*pCurrentV;
						pCurrentV=*pV+(*pCurrentT++)*3;
						v2.x=*(pCurrentV++); v2.y=*(pCurrentV++); v2.z=*pCurrentV;
						pCurrentV=*pV+(*pCurrentT)*3;
						v3.x=*(pCurrentV++); v3.y=*(pCurrentV++); v3.z=*pCurrentV;
						tVidVector &vVector1=(*pObj1).second;
						tVidVector::iterator pVector1=vVector1.begin(); // reset pList1 to v. of obj1
						while(pVector1!=vVector1.end()) // collision response
						{
							//m_nAPairs++;
							// EIT cellwise filter start ---
							//if(!CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
							//{
							//	pVector1++;
							//	continue;
							//}
							// EIT cellwise filter over ---
							//v4=(pV[1])[m_tp[1].id[0]=(pT[1])[*pVector1].p1]; //check intersection:
							//v5=(pV[1])[m_tp[1].id[1]=(pT[1])[*pVector1].p2];
							//v6=(pV[1])[m_tp[1].id[2]=(pT[1])[*pVector1].p3];
							pCurrentT=*(pT+1)+(*pVector1)*3;
							pCurrentV=*(pV+1)+(*pCurrentT++)*3;
							v4.x=*(pCurrentV++); v4.y=*(pCurrentV++); v4.z=*pCurrentV;
							pCurrentV=*(pV+1)+(*pCurrentT++)*3;
							v5.x=*(pCurrentV++); v5.y=*(pCurrentV++); v5.z=*pCurrentV;
							pCurrentV=*(pV+1)+(*pCurrentT)*3;
							v6.x=*(pCurrentV++); v6.y=*(pCurrentV++); v6.z=*pCurrentV;
							//m_nTPairs++;
							//e_performMoniter.MarkTime(&Counter);
							if( !tri_tri_intersection_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
																(float*)&v4, (float*)&v5, (float*)&v6, 
																&nCoplanar,
																(float*)&its1, (float*)&its2) 
																&& 1!=nCoplanar)
							//if( !tri_tri_overlap_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
							//						(float*)&v4, (float*)&v5, (float*)&v6) )
							//if( !NoDivTriTriIsect((float*)&v1, (float*)&v2, (float*)&v3, 
							//						(float*)&v4, (float*)&v5, (float*)&v6) )
							{
								//e_performMoniter.RecordTime(&Counter, FALSE, 'i');
								pVector1++;
								continue;
							}
							m_nPPairs++;
							// PIT cellwise filter start ---
							//if(CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
							// PIT cellwise filter over ---
							{
								//m_nDPairs++;
								nGrab=1;		
								CallResponse(nId, nType, pObjects);
							}
							pVector1++;
						}
						pVector0++;
					}
					pObj1++;			
				}
				pObj0++;
			}
		}
		p++;
	}
	}

	//// Swap CR buffers. Disable it when timing CD
	//pTissue=e_share.m_pTissueList;
	//while(pTissue)
	//{	
	//	//pTissue->deformable.ClearCRNodes();
	//	pTissue->deformable.CopyResponse();
	//	pTissue=pTissue->pNext;
	//}

	//e_share.GraspingCheck();

	return nGrab;
}

//void COMap::MeasureCP(tOMap* m_OMap)
//{
//	//static LARGE_INTEGER Counter; 	//float millisec;
//	//e_performMoniter.RecordTime(&Counter, TRUE, 'om');
//	//e_performMoniter.MarkTime(&Counter);
//
//	tCollisionSet cset[PAIR_PRIME];
//	//tCollisionSet cset;
//	m_nAPairs=m_nTPairs=m_nDPairs=0;
//
//	long i,j;
//	long nId[2]; // Objects info
//	CGeoModel* pG[2];
//	CDeformable* pD[2];
//	long nType[2];
//
//	vertex* pV[2];// Primitives info
//	vertex* pT[2];
//	long nLengthV, nLengthT;
// 	
//	vertex its1, its2; // Intersection segment
//	int nCoplanar;
//
//	vertex &v1=m_tp[0].pos[0]; // The 6 vertices of two triangles
//	vertex &v2=m_tp[0].pos[1]; 
//	vertex &v3=m_tp[0].pos[2]; 
//	vertex &v4=m_tp[1].pos[0]; 
//	vertex &v5=m_tp[1].pos[1]; 
//	vertex &v6=m_tp[1].pos[2]; 
//
//	for(long c=0; c<HASH_PRIME; c++)
//	{
//	tOMap& omap = m_OMap[c];
//	tOMap::iterator p=omap.begin();// p to cell
//	while(p!=omap.end())
//	{
//		tObjMap &obj=(*p).second; // objmap in one cell 
//		if(obj.size()>=2)
//		{
//			tObjMap::iterator pObj0=obj.begin(); // pObj to obj id/vlist
//			while(pObj0!=obj.end()) // pObj0 to obj
//			{
//				nId[0]=(*pObj0).first;
//				nType[0]=e_share.GetModel(nId[0], pG, pD);
//				pV[0]=pG[0]->GetVertex(&nLengthV);
//				pT[0]=pG[0]->GetTriangle(&nLengthT);
//				tObjMap::iterator pObj1=pObj0; pObj1++; // reset pObj1 to the next obj
//				while(pObj1!=obj.end()) // obj1 goes over
//				{
//					nId[1]=(*pObj1).first;
//					nType[1]=e_share.GetModel(nId[1], pG+1, pD+1);
//					pV[1]=pG[1]->GetVertex(&nLengthV);
//					pT[1]=pG[1]->GetTriangle(&nLengthT);
//					tVidVector &vVector0=(*pObj0).second;
//					tVidVector::iterator pVector0=vVector0.begin(); // reset pList0 to v. of obj0
//					while(pVector0!=vVector0.end())
//					{
//						v1=(pV[0])[m_tp[0].id[0]=(pT[0])[*pVector0].p1]; // check intersection:
//						v2=(pV[0])[m_tp[0].id[1]=(pT[0])[*pVector0].p2];
//						v3=(pV[0])[m_tp[0].id[2]=(pT[0])[*pVector0].p3];
//						tVidVector &vVector1=(*pObj1).second;
//						tVidVector::iterator pVector1=vVector1.begin(); // reset pList1 to v. of obj1
//						while(pVector1!=vVector1.end()) // collision response
//						{
//							m_nAPairs++;
//							// EIT cellwise filter start ---
//							if(!CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
//							{
//								pVector1++;
//								continue;
//							}
//							// EIT cellwise filter over ---
//							v4=(pV[1])[m_tp[1].id[0]=(pT[1])[*pVector1].p1]; //check intersection:
//							v5=(pV[1])[m_tp[1].id[1]=(pT[1])[*pVector1].p2];
//							v6=(pV[1])[m_tp[1].id[2]=(pT[1])[*pVector1].p3];
//							m_nTPairs++;
//							//e_performMoniter.MarkTime(&Counter);
//							if( !tri_tri_intersection_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//																(float*)&v4, (float*)&v5, (float*)&v6, 
//																&nCoplanar,
//																(float*)&its1, (float*)&its2) )
//							//if( !tri_tri_overlap_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							//if( !NoDivTriTriIsect((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							{
//								//e_performMoniter.RecordTime(&Counter, FALSE, 'i');
//								pVector1++;
//								continue;
//							}
//							m_nDPairs++;
//							pVector1++;
//						}
//						pVector0++;
//					}
//					pObj1++;			
//				}
//				pObj0++;
//			}
//		}
//		p++;
//	}
//	}
//}

//inline void COMap::CallResponse(long id[], long nType[], CDeformable* pD[])
//{
//	int nCollisionType=-1;
//	vertex velocity[2];
//	tCRNode* pCRNodes, *pCRNodes1;
//	tInstrumentLink* pInstrument1;
//	long nWhichI, nWhichR, nWhichT, nWhichN, nWhichS;
//	bool bMedial;
//	vertex loc;
//	long i;
//
//		//vertex adjust0, adjust1, loc;
//		//	vertex v[3][2];
//		//	float dx[3][3];
//
//	CalcNormal(m_tp[0].pos, m_tp[0].pos+1, m_tp[0].pos+2, &(m_tp[0].n));
//	CalcNormal(m_tp[1].pos, m_tp[1].pos+1, m_tp[1].pos+2, &(m_tp[1].n));
//
//	// Instrument involved
//	if(nType[0] == INSTRUMENT_TYPE || nType[1] == INSTRUMENT_TYPE)
//	{
//		if(nType[0] == INSTRUMENT_TYPE && nType[1] == INSTRUMENT_TYPE)
//		{
//			nCollisionType=I2I;
//		}
//		else if(nType[0] == INSTRUMENT_TYPE && nType[1] == TISSUE_TYPE)
//		{
//			nWhichI=0;
//			nWhichT=1;
//			nCollisionType=I2T;
//		}
//		else if(nType[0] == TISSUE_TYPE && nType[1] == INSTRUMENT_TYPE)
//		{
//			nWhichI=1;
//			nWhichT=0;
//			nCollisionType=I2T;
//		}
//	}// Tissue to tissue 
//	else if(nType[0] == TISSUE_TYPE && nType[1] == TISSUE_TYPE)
//	{
//		nCollisionType=T2T;
//	}
//
//	switch(nCollisionType)
//	{
//		case I2T:
//		{
//			if( NULL == (pInstrument1=e_share.GetInstrument(id[nWhichI])) ) return;
//			pInstrument1->geoModel.GetDisplacement(m_tp[nWhichI].id[0], m_tp[nWhichI].v);
//			pInstrument1->geoModel.GetDisplacement(m_tp[nWhichI].id[1], m_tp[nWhichI].v+1);
//			pInstrument1->geoModel.GetDisplacement(m_tp[nWhichI].id[2], m_tp[nWhichI].v+2);
//			//m_tp[nWhichI].v[0]=pInstrument1->velocity;
//			//m_tp[nWhichI].v[1]=pInstrument1->velocity;
//			//m_tp[nWhichI].v[2]=pInstrument1->velocity;
//			m_tp[nWhichT].v[0]=(pD[nWhichT]->m_CurrentSys)[m_tp[nWhichT].id[0]].v;
//			m_tp[nWhichT].v[1]=(pD[nWhichT]->m_CurrentSys)[m_tp[nWhichT].id[1]].v;
//			m_tp[nWhichT].v[2]=(pD[nWhichT]->m_CurrentSys)[m_tp[nWhichT].id[2]].v;
//
//			bMedial = pInstrument1->geoModel.IsMedial(m_tp[nWhichI].id[0]);
//			bMedial |= pInstrument1->geoModel.IsMedial(m_tp[nWhichI].id[1]);
//			bMedial |= pInstrument1->geoModel.IsMedial(m_tp[nWhichI].id[2]);
//			if(bMedial && e_share.m_graspStatus[id[nWhichI]]==1)
//			{
//				long &nCount=pD[nWhichT]->m_response_b.nCPos[id[nWhichI]];
//				pCRNodes=pD[nWhichT]->m_response_b.CPos[id[nWhichI]];
//				for(i=0;i<3;i++)
//				{
//					if( nCount< CNODES_MAX && IsNewCRNode(m_tp[nWhichT].id[i], FALSE, pD[nWhichT]))
//					{
//						VectorDifference(m_tp[nWhichT].pos+i, m_tp[nWhichI].pos+i, &(pCRNodes[nCount].vector));
//						pCRNodes[nCount].id=m_tp[nWhichT].id[i];
//						pCRNodes[nCount].id_instrument=id[nWhichI];
//						pCRNodes[nCount++].id_iNode=m_tp[nWhichI].id[i];
//					}
//				}
//			}
//			else{
//				long &nCount=pD[1]->m_response_b.nRVelocity;
//				pCRNodes=pD[1]->m_response_b.RVelocity;
//				long &nCount1=pD[1]->m_response_b.nPPos;
//				pCRNodes1=pD[1]->m_response_b.PPos;
//				float fLength;
//				vertex vn, vt, v;
//				for(i=0;i<3;i++)
//				{
//					if(nCount < RNODES_MAX && nCount1 < RNODES_MAX)
//					{
//						v=m_tp[nWhichT].v[i];
//						vn=m_tp[nWhichI].n;
//						VectorDifference(&v, m_tp[nWhichI].v+i, &loc);
//						fLength=DotProduct(&loc, &vn);
//						if(0.0f<=fLength) continue;
//						ScaleVector(&vn, fLength, &vn);
//						VectorDifference(&v, &vn, &vt);
//
//						ScaleVector(&vn, 0.5f, &vn); //0.5f
//						VectorDifference(&vt, &vn, &loc);
//						//VectorDifference(&loc, &v, &loc);
//
//						//ScaleVector(&vt, 0.6f, &loc);
//
//						//ScaleVector(&loc, 0.5f, &loc);
//						pCRNodes[nCount].vector=loc;
//						//VectorSum(&(pCRNodes[nCount].vector),&loc,&(pCRNodes[nCount].vector));
//						pCRNodes[nCount].id_instrument=id[nWhichI];
//						//pCRNodes[nCount].id_iNode=m_tp[nWhichI].id[i];
//						pCRNodes[nCount++].id=m_tp[nWhichT].id[i];
//
//					/*	VectorSum(m_tp[nWhichT].pos+i, &(m_tp[nWhichI].n), &(pCRNodes2[nCount1].vector));
//						//pCRNodes2[nCount1].vector=m_tp[nWhichI].pos[i];
//						pCRNodes2[nCount1].id_instrument=id[nWhichI];
//						pCRNodes2[nCount1++].id=m_tp[nWhichT].id[i];*/
//					}
//				}
//			}
//			//CColliResp::CollisionResponse(I2T, nWhichI, nWhichR, nWhichT);
//			break;
//		}
//		case T2T:
//		{
//			if(!CR_TT) break;
//			float fLength;
//			vertex vn, vt, v, vs;
//			vertex* pvs;
//
//			nWhichN=0;
//			nWhichS=1;
//			
//			long &nCount0=pD[nWhichN]->m_response_b.nRVelocity;
//			pCRNodes=pD[nWhichN]->m_response_b.RVelocity;
//
//			pvs=m_tp[nWhichS].v;
//			vs=*(pvs);
//			VectorSum(&vs,pvs+1,&vs);
//			VectorSum(&vs,pvs+2,&vs);
//			ScaleVector(&vs,1.0/3.0f,&vs);
//			
//			for(i=0;i<3;i++)
//			{
//				if(nCount0 < RNODES_MAX && IsNewCRNode(m_tp[nWhichN].id[i], TRUE, pD[nWhichN]))
//				{
//					v=m_tp[nWhichN].v[i];
//					vn=m_tp[nWhichS].n;
//					VectorDifference(&v, &vs, &loc);
//					fLength=DotProduct(&loc, &vn);
//					if(0.0f<=fLength) continue;
//					ScaleVector(&vn, fLength, &vn);
//					VectorDifference(&v, &vn, &vt);
//
//					ScaleVector(&vn, 0.1f, &vn);
//					VectorDifference(&vt, &vn, &loc); //VectorDifference(&loc, &v, &loc);
//					//ScaleVector(&vt, 0.6f, &loc);
//					ScaleVector(&loc, 0.1f, &loc); //0.1f
//					pCRNodes[nCount0].vector=loc; //VectorSum(&(pCRNodes[nCount].vector),&loc,&(pCRNodes[nCount].vector));
//					pCRNodes[nCount0++].id=m_tp[nWhichN].id[i];
//				}
//			}
//
//			nWhichN=1;
//			nWhichS=0;
//			
//			long &nCount1=pD[nWhichN]->m_response_b.nRVelocity;
//			pCRNodes=pD[nWhichN]->m_response_b.RVelocity;
//
//			pvs=m_tp[nWhichS].v;
//			vs=*(pvs);
//			VectorSum(&vs,pvs+1,&vs);
//			VectorSum(&vs,pvs+2,&vs);
//			ScaleVector(&vs,1.0/3.0f,&vs);
//			
//			for(i=0;i<3;i++)
//			{
//				if(nCount1 < RNODES_MAX && IsNewCRNode(m_tp[nWhichN].id[i], TRUE, pD[nWhichN]))
//				{
//					v=m_tp[nWhichN].v[i];
//					vn=m_tp[nWhichS].n;
//					VectorDifference(&v, &vs, &loc);
//					fLength=DotProduct(&loc, &vn);
//					if(0.0f<=fLength) continue;
//					ScaleVector(&vn, fLength, &vn);
//					VectorDifference(&v, &vn, &vt);
//
//					ScaleVector(&vn, 0.5f, &vn);
//					VectorDifference(&vt, &vn, &loc); //VectorDifference(&loc, &v, &loc);
//					//ScaleVector(&vt, 0.6f, &loc);
//					ScaleVector(&loc, 0.1f, &loc); //0.1f
//					pCRNodes[nCount1].vector=loc; //VectorSum(&(pCRNodes[nCount].vector),&loc,&(pCRNodes[nCount].vector));
//					pCRNodes[nCount1++].id=m_tp[nWhichN].id[i];
//				}
//			}
//			break;
//		}
//	}
//}


inline void COMap::CallResponse(long id[], long nType[], CrestObjects *pObjects)
{
	int nCollisionType=-1;
	vertex velocity[2];
	tCRNode* pCRNodes, *pCRNodes1;
	CrestInstrumentInfo *pIInfo;//tInstrumentLink* pInstrument1;
	ResponseData *pResponse;
	long nWhichI, nWhichR, nWhichT, nWhichN, nWhichS;
	bool bMedial;
	vertex loc;
	long i;

		//vertex adjust0, adjust1, loc;
		//	vertex v[3][2];
		//	float dx[3][3];

	CalcNormal(m_tp[0].pos, m_tp[0].pos+1, m_tp[0].pos+2, &(m_tp[0].n));
	CalcNormal(m_tp[1].pos, m_tp[1].pos+1, m_tp[1].pos+2, &(m_tp[1].n));

	// Instrument involved
	if(nType[0] == INSTRUMENT_TYPE || nType[1] == INSTRUMENT_TYPE)
	{
		if(nType[0] == INSTRUMENT_TYPE && nType[1] == INSTRUMENT_TYPE)
		{
			nCollisionType=I2I;
		}
		else if(nType[0] == INSTRUMENT_TYPE && nType[1] == TISSUE_TYPE)
		{
			nWhichI=0;
			nWhichT=1;
			nCollisionType=I2T;
		}
		else if(nType[0] == TISSUE_TYPE && nType[1] == INSTRUMENT_TYPE)
		{
			nWhichI=1;
			nWhichT=0;
			nCollisionType=I2T;
		}
	}// Tissue to tissue 
	else if(nType[0] == TISSUE_TYPE && nType[1] == TISSUE_TYPE)
	{
		nCollisionType=T2T;
	}

	switch(nCollisionType)
	{
		case I2T:
		{
			if( NULL == (pIInfo = pObjects->GetObjectInfo(id[nWhichI])->pIInfo) ) return;
			if( NULL == (pResponse = pObjects->GetObjectInfo(id[nWhichT])->pResponse) ) return;
			//pInstrument1->geoModel.GetDisplacement(m_tp[nWhichI].id[0], m_tp[nWhichI].v);
			//pInstrument1->geoModel.GetDisplacement(m_tp[nWhichI].id[1], m_tp[nWhichI].v+1);
			//pInstrument1->geoModel.GetDisplacement(m_tp[nWhichI].id[2], m_tp[nWhichI].v+2);
			//	//m_tp[nWhichI].v[0]=pInstrument1->velocity;
			//	//m_tp[nWhichI].v[1]=pInstrument1->velocity;
			//	//m_tp[nWhichI].v[2]=pInstrument1->velocity;
			//m_tp[nWhichT].v[0]=(pD[nWhichT]->m_CurrentSys)[m_tp[nWhichT].id[0]].v;
			//m_tp[nWhichT].v[1]=(pD[nWhichT]->m_CurrentSys)[m_tp[nWhichT].id[1]].v;
			//m_tp[nWhichT].v[2]=(pD[nWhichT]->m_CurrentSys)[m_tp[nWhichT].id[2]].v;

			bMedial = pIInfo->IsMedial(m_tp[nWhichI].id[0]);
			bMedial |= pIInfo->IsMedial(m_tp[nWhichI].id[1]);
			bMedial |= pIInfo->IsMedial(m_tp[nWhichI].id[2]);
			if(bMedial && pIInfo->nGraspStatus==1)
			{
				long &nCount=pResponse->nCPos[id[nWhichI]];
				pCRNodes=pResponse->CPos[id[nWhichI]];
				for(i=0;i<3;i++)
				{
					if( nCount< CNODES_MAX && pResponse->IsNewCRNode(m_tp[nWhichT].id[i], false))
					{
						VectorDifference(m_tp[nWhichT].pos+i, m_tp[nWhichI].pos+i, &(pCRNodes[nCount].vector));
						pCRNodes[nCount].id=m_tp[nWhichT].id[i];
						pCRNodes[nCount].id_instrument=id[nWhichI];
						pCRNodes[nCount++].id_iNode=m_tp[nWhichI].id[i];
					}
				}
			}
			//else{
			//	long &nCount=pD[1]->m_response_b.nRVelocity;
			//	pCRNodes=pD[1]->m_response_b.RVelocity;
			//	long &nCount1=pD[1]->m_response_b.nPPos;
			//	pCRNodes1=pD[1]->m_response_b.PPos;
			//	CDFloat fLength;
			//	vertex vn, vt, v;
			//	for(i=0;i<3;i++)
			//	{
			//		if(nCount < RNODES_MAX && nCount1 < RNODES_MAX)
			//		{
			//			v=m_tp[nWhichT].v[i];
			//			vn=m_tp[nWhichI].n;
			//			VectorDifference(&v, m_tp[nWhichI].v+i, &loc);
			//			fLength=DotProduct(&loc, &vn);
			//			if(0.0f<=fLength) continue;
			//			ScaleVector(&vn, fLength, &vn);
			//			VectorDifference(&v, &vn, &vt);

			//			ScaleVector(&vn, 0.5f, &vn); //0.5f
			//			VectorDifference(&vt, &vn, &loc);
			//			//VectorDifference(&loc, &v, &loc);

			//			//ScaleVector(&vt, 0.6f, &loc);

			//			//ScaleVector(&loc, 0.5f, &loc);
			//			pCRNodes[nCount].vector=loc;
			//			//VectorSum(&(pCRNodes[nCount].vector),&loc,&(pCRNodes[nCount].vector));
			//			pCRNodes[nCount].id_instrument=id[nWhichI];
			//			//pCRNodes[nCount].id_iNode=m_tp[nWhichI].id[i];
			//			pCRNodes[nCount++].id=m_tp[nWhichT].id[i];

			//		/*	VectorSum(m_tp[nWhichT].pos+i, &(m_tp[nWhichI].n), &(pCRNodes2[nCount1].vector));
			//			//pCRNodes2[nCount1].vector=m_tp[nWhichI].pos[i];
			//			pCRNodes2[nCount1].id_instrument=id[nWhichI];
			//			pCRNodes2[nCount1++].id=m_tp[nWhichT].id[i];*/
			//		}
			//	}
			//}
			//	//CColliResp::CollisionResponse(I2T, nWhichI, nWhichR, nWhichT);
			break;
		}
		//case T2T:
		//{
		//	if(!CR_TT) break;
		//	float fLength;
		//	vertex vn, vt, v, vs;
		//	vertex* pvs;

		//	nWhichN=0;
		//	nWhichS=1;
		//	
		//	long &nCount0=pD[nWhichN]->m_response_b.nRVelocity;
		//	pCRNodes=pD[nWhichN]->m_response_b.RVelocity;

		//	pvs=m_tp[nWhichS].v;
		//	vs=*(pvs);
		//	VectorSum(&vs,pvs+1,&vs);
		//	VectorSum(&vs,pvs+2,&vs);
		//	ScaleVector(&vs,1.0/3.0f,&vs);
		//	
		//	for(i=0;i<3;i++)
		//	{
		//		if(nCount0 < RNODES_MAX && IsNewCRNode(m_tp[nWhichN].id[i], TRUE, pD[nWhichN]))
		//		{
		//			v=m_tp[nWhichN].v[i];
		//			vn=m_tp[nWhichS].n;
		//			VectorDifference(&v, &vs, &loc);
		//			fLength=DotProduct(&loc, &vn);
		//			if(0.0f<=fLength) continue;
		//			ScaleVector(&vn, fLength, &vn);
		//			VectorDifference(&v, &vn, &vt);

		//			ScaleVector(&vn, 0.1f, &vn);
		//			VectorDifference(&vt, &vn, &loc); //VectorDifference(&loc, &v, &loc);
		//			//ScaleVector(&vt, 0.6f, &loc);
		//			ScaleVector(&loc, 0.1f, &loc); //0.1f
		//			pCRNodes[nCount0].vector=loc; //VectorSum(&(pCRNodes[nCount].vector),&loc,&(pCRNodes[nCount].vector));
		//			pCRNodes[nCount0++].id=m_tp[nWhichN].id[i];
		//		}
		//	}

		//	nWhichN=1;
		//	nWhichS=0;
		//	
		//	long &nCount1=pD[nWhichN]->m_response_b.nRVelocity;
		//	pCRNodes=pD[nWhichN]->m_response_b.RVelocity;

		//	pvs=m_tp[nWhichS].v;
		//	vs=*(pvs);
		//	VectorSum(&vs,pvs+1,&vs);
		//	VectorSum(&vs,pvs+2,&vs);
		//	ScaleVector(&vs,1.0/3.0f,&vs);
		//	
		//	for(i=0;i<3;i++)
		//	{
		//		if(nCount1 < RNODES_MAX && IsNewCRNode(m_tp[nWhichN].id[i], TRUE, pD[nWhichN]))
		//		{
		//			v=m_tp[nWhichN].v[i];
		//			vn=m_tp[nWhichS].n;
		//			VectorDifference(&v, &vs, &loc);
		//			fLength=DotProduct(&loc, &vn);
		//			if(0.0f<=fLength) continue;
		//			ScaleVector(&vn, fLength, &vn);
		//			VectorDifference(&v, &vn, &vt);

		//			ScaleVector(&vn, 0.5f, &vn);
		//			VectorDifference(&vt, &vn, &loc); //VectorDifference(&loc, &v, &loc);
		//			//ScaleVector(&vt, 0.6f, &loc);
		//			ScaleVector(&loc, 0.1f, &loc); //0.1f
		//			pCRNodes[nCount1].vector=loc; //VectorSum(&(pCRNodes[nCount].vector),&loc,&(pCRNodes[nCount].vector));
		//			pCRNodes[nCount1++].id=m_tp[nWhichN].id[i];
		//		}
		//	}
		//	break;
		//}
	}
}

//void COMap::FillCountersParti(tCounterMap& counterMap, CPartiSys& partisys)
//{
//	long nParticleCount=partisys.m_emitter.particleCount;
//	tParti* pParti=partisys.m_emitter.particle;
//	long nV;
//	vertex* pV=partisys.m_geoModel.GetVertex(&nV);
//
//	vertex vp[4]; // vertices of 2-triangle primitive - folded rectangle
//	long idT; 
//	long i, n, t, nOffset;
//	DWORD dwCell;
//
//	n=nOffset=INSTRUMENT_MAX + RIGID_MAX + TISSUE_MAX;
//	while (pParti)
//	{
//		*(vp)=*(pV++);
//		*(vp+1)=*(pV++);
//		*(vp+2)=*(pV++);
//		*(vp+3)=*(pV++);
//		RasterAABB(vp, 4, m_idTCell, m_nCountTCell, RASTER_SIZE);
//
//		//t=(n-nOffset)*2; // one rectangle primtive, or 2 triangles
//		//tPrimCell &pc=m_opc[n];
//		//tCidVector &cid1=pc[t];
//		//tCidVector &cid2=pc[t+1];
//		//cid1.clear();
//		//cid2.clear();
//		//for(i=0;i<m_nCountTCell;i++)
//		//{
//		//	cid1.push_back(*(m_idTCell+i));
//		//	cid2.push_back(*(m_idTCell+i));
//		//}
//
//		for(i=0;i<m_nCountTCell;i++)
//		{
//			dwCell=*(m_idTCell+i);
//			tCo* pCo=&counterMap[dwCell];
//			if(pCo->first<1){ // new
//				pCo->first=1;
//				pCo->second=n;
//			}
//			else if(pCo->second!=n)
//			{
//				pCo->first++;
//				pCo->second=n;
//			}
//		}
//		n++; // particle temporary id for counterMap only
//		pParti = pParti->next;
//	}
//}
//
//void COMap::FillOMapParti(tOMap* m_OMap, CPartiSys& partisys)
//{
//	long nParticleCount=partisys.m_emitter.particleCount;
//	tParti* pParti=partisys.m_emitter.particle;
//	long nV;
//	vertex* pV=partisys.m_geoModel.GetVertex(&nV);
//
//	vertex vp[4]; // vertices of 2-triangle primitive - folded rectangle
//	long idT; 
//	long i, n, t, nOffset;
//	DWORD dwCell;
//
//	n=nOffset=INSTRUMENT_MAX + RIGID_MAX + TISSUE_MAX;
//	while (pParti)
//	{
//		*(vp)=*(pV++);
//		*(vp+1)=*(pV++);
//		*(vp+2)=*(pV++);
//		*(vp+3)=*(pV++);
//		RasterAABB(vp, 4, m_idTCell, m_nCountTCell, RASTER_SIZE);
//
//		t=(n-nOffset)*2; // one rectangle primtive, or 2 triangles
//		for(i=0;i<m_nCountTCell;i++)
//		{
//			FillCell(m_OMap, n, t, *(m_idTCell+i));
//			FillCell(m_OMap, n, t+1, *(m_idTCell+i));
//		}
//		
//		n++; // particle temporary id for counterMap only
//		pParti = pParti->next;
//	}
//}
//
//void COMap::FillOMapParti(tOMap* m_OMap)
//{
//	long n, t;
//
//	tObjPrimCell::iterator itopc=m_opc.begin();
//	while(itopc!=m_opc.end())
//	{
//		n=(*itopc).first;
//		tPrimCell &pc=(*itopc).second;
//		tPrimCell::iterator itpc=pc.begin();
//		while(itpc!=pc.end())
//		{
//			t=(*itpc).first;
//			tCidVector &cid=(*itpc).second;
//			tCidVector::iterator itcid=cid.begin();
//			while(itcid!=cid.end())
//			{
//				FillCell(m_OMap, n, t, *(itcid));
//				itcid++;
//			}
//			itpc++;
//		}
//		itopc++;
//	}		
//}
//
//
//int COMap::DetectCRParti(tOMap* m_OMap)
//{
//	//static LARGE_INTEGER Counter; 	//float millisec;
//	//e_performMoniter.RecordTime(&Counter, TRUE, 'om');
//	//e_performMoniter.MarkTime(&Counter);
//
//	//tCollisionSet cset[PAIR_PRIME];
//	////tCollisionSet cset;
//	//m_nAPairs=m_nTPairs=m_nPPairs=0;
//	//m_nPPairs=m_nDPairs=0;
//	m_nPPairs=0;
//	
//	long i,j;
//
//	long nId[2]; // Objects info
//	CGeoModel* pG=&(e_share.m_partiSys.m_geoModel);
//	CGeoModel* pD;
//	long nType[2]; 
//
//	vertex* pV[2];// Primitives info
//	vertex* pT[2];
//	long nLengthV, nLengthT;
// 	
//	vertex its1, its2; // Intersection segment
//	int nCoplanar;
//
//	int nGrab=-1;
//
//	vertex &v1=m_tp[0].pos[0]; // The 6 vertices of two triangles
//	vertex &v2=m_tp[0].pos[1]; 
//	vertex &v3=m_tp[0].pos[2]; 
//	vertex &v4=m_tp[1].pos[0]; 
//	vertex &v5=m_tp[1].pos[1]; 
//	vertex &v6=m_tp[1].pos[2]; 
//
//	pV[0]=pG->GetVertex(&nLengthV);
//	pT[0]=pG->GetTriangle(&nLengthT);
//
//	for(long c=0; c<HASH_PRIME; c++)
//	{
//	tOMap& omap = m_OMap[c];
//	tOMap::iterator p=omap.begin();// p to cell
//	while(p!=omap.end())
//	{
//		tObjMap &obj=(*p).second; // objmap in one cell 
//		if(obj.size()>=2)
//		{
//			tObjMap::iterator pObj0=obj.begin(); // pObj to obj id/vlist
//			while(pObj0!=obj.end()) // pObj0 to obj
//			{
//				nId[0]=(*pObj0).first;
//				nType[0]=PARTI_TYPE;
//				//pV[0]=pG->GetVertex(&nLengthV);
//				//pT[0]=pG->GetTriangle(&nLengthT);
//				tObjMap::iterator pObj1=pObj0; pObj1++; // reset pObj1 to the next obj
//				while(pObj1!=obj.end()) // obj1 goes over
//				{
//					nId[1]=(*pObj1).first;
//					nType[1]=PARTI_TYPE;
//					pV[1]=pV[0];
//					pT[1]=pT[0];
//					tVidVector &vVector0=(*pObj0).second;
//					tVidVector::iterator pVector0=vVector0.begin(); // reset pList0 to v. of obj0
//					while(pVector0!=vVector0.end())
//					{
//						v1=(pV[0])[m_tp[0].id[0]=(pT[0])[*pVector0].p1]; // check intersection:
//						v2=(pV[0])[m_tp[0].id[1]=(pT[0])[*pVector0].p2];
//						v3=(pV[0])[m_tp[0].id[2]=(pT[0])[*pVector0].p3];
//						tVidVector &vVector1=(*pObj1).second;
//						tVidVector::iterator pVector1=vVector1.begin(); // reset pList1 to v. of obj1
//						while(pVector1!=vVector1.end()) // collision response
//						{
//							//m_nAPairs++;
//							// EIT cellwise filter start ---
//							//if(!CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
//							//{
//							//	pVector1++;
//							//	continue;
//							//}
//							// EIT cellwise filter over ---
//							v4=(pV[1])[m_tp[1].id[0]=(pT[1])[*pVector1].p1]; //check intersection:
//							v5=(pV[1])[m_tp[1].id[1]=(pT[1])[*pVector1].p2];
//							v6=(pV[1])[m_tp[1].id[2]=(pT[1])[*pVector1].p3];
//							//m_nTPairs++;
//							//e_performMoniter.MarkTime(&Counter);
//							if( !tri_tri_intersection_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//																(float*)&v4, (float*)&v5, (float*)&v6, 
//																&nCoplanar,
//																(float*)&its1, (float*)&its2) )
//							//if( !tri_tri_overlap_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							//if( !NoDivTriTriIsect((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							{
//								//e_performMoniter.RecordTime(&Counter, FALSE, 'i');
//								pVector1++;
//								continue;
//							}
//							m_nPPairs++;
//							// PIT cellwise filter start ---
//							//if(CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
//							// PIT cellwise filter over ---
//							{
//								//m_nDPairs++;
//								nGrab=1;		
//								//CallResponse(nId, nType, pD);
//							}
//							pVector1++;
//						}
//						pVector0++;
//					}
//					pObj1++;			
//				}
//				pObj0++;
//			}
//		}
//		p++;
//	}
//	}
//
//	// Swap CR buffers. Disable it when timing CD
//	//pTissue=e_share.m_pTissueList;
//	//while(pTissue)
//	//{	
//	//	//pTissue->deformable.ClearCRNodes();
//	//	pTissue->deformable.CopyResponse();
//	//	pTissue=pTissue->pNext;
//	//}
//
//	return e_mi.m_iGrab=nGrab;
//}
//
//void COMap::MeasureCPParti(tOMap* m_OMap)
//{
//	//static LARGE_INTEGER Counter; 	//float millisec;
//	//e_performMoniter.RecordTime(&Counter, TRUE, 'om');
//	//e_performMoniter.MarkTime(&Counter);
//
//	tCollisionSet cset[PAIR_PRIME];
//	//tCollisionSet cset;
//	m_nAPairs=m_nTPairs=m_nDPairs=0;
//
//	long i,j;
//
//	long nId[2]; // Objects info
//	CGeoModel* pG=&(e_share.m_partiSys.m_geoModel);
//	CGeoModel* pD;
//	long nType[2]; 
//
//	vertex* pV[2];// Primitives info
//	vertex* pT[2];
//	long nLengthV, nLengthT;
// 	
//	vertex its1, its2; // Intersection segment
//	int nCoplanar;
//
//	vertex &v1=m_tp[0].pos[0]; // The 6 vertices of two triangles
//	vertex &v2=m_tp[0].pos[1]; 
//	vertex &v3=m_tp[0].pos[2]; 
//	vertex &v4=m_tp[1].pos[0]; 
//	vertex &v5=m_tp[1].pos[1]; 
//	vertex &v6=m_tp[1].pos[2]; 
//
//	pV[0]=pG->GetVertex(&nLengthV);
//	pT[0]=pG->GetTriangle(&nLengthT);
//
//	for(long c=0; c<HASH_PRIME; c++)
//	{
//	tOMap& omap = m_OMap[c];
//	tOMap::iterator p=omap.begin();// p to cell
//	while(p!=omap.end())
//	{
//		tObjMap &obj=(*p).second; // objmap in one cell 
//		if(obj.size()>=2)
//		{
//			tObjMap::iterator pObj0=obj.begin(); // pObj to obj id/vlist
//			while(pObj0!=obj.end()) // pObj0 to obj
//			{
//				nId[0]=(*pObj0).first;
//				nType[0]=PARTI_TYPE;
//				//pV[0]=pG->GetVertex(&nLengthV);
//				//pT[0]=pG->GetTriangle(&nLengthT);
//				tObjMap::iterator pObj1=pObj0; pObj1++; // reset pObj1 to the next obj
//				while(pObj1!=obj.end()) // obj1 goes over
//				{
//					nId[1]=(*pObj1).first;
//					nType[1]=PARTI_TYPE;
//					pV[1]=pV[0];
//					pT[1]=pT[0];
//					tVidVector &vVector0=(*pObj0).second;
//					tVidVector::iterator pVector0=vVector0.begin(); // reset pList0 to v. of obj0
//					while(pVector0!=vVector0.end())
//					{
//						v1=(pV[0])[m_tp[0].id[0]=(pT[0])[*pVector0].p1]; // check intersection:
//						v2=(pV[0])[m_tp[0].id[1]=(pT[0])[*pVector0].p2];
//						v3=(pV[0])[m_tp[0].id[2]=(pT[0])[*pVector0].p3];
//						tVidVector &vVector1=(*pObj1).second;
//						tVidVector::iterator pVector1=vVector1.begin(); // reset pList1 to v. of obj1
//						while(pVector1!=vVector1.end()) // collision response
//						{
//							m_nAPairs++;
//							// EIT cellwise filter start ---
//							if(!CellwiseFilter(cset,*nId,*(nId+1),*pVector0,*pVector1)) // if(!CellwiseFilter(&cset,*nId[0],*(nId+1),*pVector0,*pVector1))
//							{
//								pVector1++;
//								continue;
//							}
//							// EIT cellwise filter over ---
//							v4=(pV[1])[m_tp[1].id[0]=(pT[1])[*pVector1].p1]; //check intersection:
//							v5=(pV[1])[m_tp[1].id[1]=(pT[1])[*pVector1].p2];
//							v6=(pV[1])[m_tp[1].id[2]=(pT[1])[*pVector1].p3];
//							m_nTPairs++;
//							//e_performMoniter.MarkTime(&Counter);
//							if( !tri_tri_intersection_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//																(float*)&v4, (float*)&v5, (float*)&v6, 
//																&nCoplanar,
//																(float*)&its1, (float*)&its2) )
//							//if( !tri_tri_overlap_test_3d((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							//if( !NoDivTriTriIsect((float*)&v1, (float*)&v2, (float*)&v3, 
//							//						(float*)&v4, (float*)&v5, (float*)&v6) )
//							{
//								//e_performMoniter.RecordTime(&Counter, FALSE, 'i');
//								pVector1++;
//								continue;
//							}
//							m_nDPairs++;
//							//CallResponse(nId, nType, pD);
//							pVector1++;
//						}
//						pVector0++;
//					}
//					pObj1++;			
//				}
//				pObj0++;
//			}
//		}
//		p++;
//	}
//	}
//}

inline DWORD COMap::Coor2dw(vertex& v)
{
	//return (DWORD)(v.x/m_dwR+500)+(DWORD)(v.y/m_dwR+500)*1000+(DWORD)(v.z/m_dwR+500)*1000000; // [-500,500]
	return (Real2Int(v.x)/m_dwR+500+(Real2Int(v.y)/m_dwR+500)*1000+(Real2Int(v.z)/m_dwR+500)*1000000); // [-500,500]
}

inline DWORD COMap::Hash(DWORD dwCell)
{
	//DWORD tmp=dwCell;
	//tmp=tmp>>29;
	//dwCell=dwCell<<3;
	//return (dwCell|=tmp)%HASH_PRIME;
	return dwCell%HASH_PRIME;
}

inline void COMap::PairIndex(LONG_ID& nPair, long nObj1, long nObj2, long nV1, long nV2)
{
	nPair=0;
	switch(PAIR_INDEX_MODE)
	{
		case 1: { // stapling // 64t 64r 8i 2048p 
				nPair|=nV1;
				nPair<<=8;
				nPair|=nObj2;
				nPair<<=11;
				nPair|=nV2;
				nPair<<=8;
				nPair|=nObj1;
				break;
			}
		case 2: { // 2 big // 64t 64r 8i 131072p : obj 8 bits, p 23 bits
				nPair|=nV1;
				nPair<<=8;
				nPair|=nObj2;
				nPair<<=17;
				nPair|=nV2;
				nPair<<=8;
				nPair|=nObj1;
				break;
			}
		case 3: { // parti // 64t 64r 8i 32???parti 65536p : obj 11 bits, p 11 bits 
				nPair|=nV1;
				nPair<<=15;
				nPair|=nObj2;
				nPair<<=16;
				nPair|=nV2;
				nPair<<=15;
				nPair|=nObj1;
				break;
				}
		default: { // 32768 obj; 131072 prim
				nPair|=nV1;
				nPair<<=15;
				nPair|=nObj2;
				nPair<<=17;
				nPair|=nV2;
				nPair<<=15;
				nPair|=nObj1;
			}
	}

	// 20t 8r 4i 2048p 
	//nPair|=nV1;
	//nPair<<=5;
	//nPair|=nObj2;
	//nPair<<=11;
	//nPair|=nV2;
	//nPair<<=5;
	//nPair|=nObj1;
	//nPair=((nObj1*2048+nV1)*32+nObj2)*2048+nV2;
}

//inline COMap::PairIndex(LONG_ID& nPair, long nObj1, long nObj2, long nV1, long nV2)
//{
//	DWORD first, second;
//	first|=nV1; // 64t 64r 8i 32768p : obj 8 bits, p 23 bits 
//	first<<=8;
//	first|=nObj1;
//	second|=nV2;
//	second<<=8;
//	second|=nObj2;
//
//	nPair.first=first;
//	nPair.second=second;
//}

inline DWORD COMap::PairHash(LONG_ID nPair)
{
	//LONG_ID tmp=nPair;
	//tmp=tmp>>24;
	//nPair=nPair<<8;
	//return (nPair|=tmp)%HASH_PRIME;
	return nPair%PAIR_PRIME;
}

inline bool COMap::CellwiseFilter(tCollisionSet* cset, long& nId0, long& nId1, long& nVid0, long& nVid1)
{
	tCollisionSet::iterator itPair;
	LONG_ID nPair;
	DWORD dwPairEntry;
	bool bNew;

	PairIndex(nPair,nId0,nId1,nVid0,nVid1); 
	dwPairEntry=PairHash(nPair);
	itPair=cset[dwPairEntry].find(nPair);
	if( bNew = itPair==cset[dwPairEntry].end() )
		cset[dwPairEntry].insert(nPair);
	//if( bNew = itPair==cset->end() )
		//cset->insert(nPair);

	return bNew;
}