//#include "stdafx.h"
#include "ColliResp.h"

//#include "Share.h"
#include "OMap.h"
#include "MathDefs.h"
//#include "Deformable.h"
//#include "GeoModel.h"
//#include "ExternDeclare.h"
#include "RayTri.h"



CColliResp::CColliResp(void)
{
}

CColliResp::~CColliResp(void)
{
}

void CColliResp::CollisionResponse(long nCollisionType, long nWhichI, long nWhichR, long nWhichT, tTrianglePhy tp[])
{
	switch(nCollisionType)
	{
	case I2T:
		{
			break;
		}
	case T2T:
		{
			break;
		}
	}
}

void CColliResp::ItoTObject(CrestObjects *pObjects)
{
	const long nObjects=pObjects->m_nObjects;
	CrestObjectInfoV& ObjectInfoV=pObjects->m_ObjectInfo;
	if(nObjects!=ObjectInfoV.size()) return;
	
	vertex *pV[OBJECT_MAX], *pVT, *pVI; 
	float* pN[OBJECT_MAX];
	long *pT[OBJECT_MAX], *pTT, *pTI;
	long nT[OBJECT_MAX], nV[OBJECT_MAX], nVT, nTT, nVI, nTI;
	float* pNT, *pNI; //vertex* pNT;
	vertex *pDT, DI;
	
	for(long i=0; i<nObjects; i++)
	{
		CrestObjectInfo& ObjInfo = ObjectInfoV[i];
		long& nID=ObjInfo.nID;
		*(pV+nID)=(vertex*)ObjInfo.pVertex; // For V check
		*(nV+nID)=ObjInfo.nVertex;
		*(pT+nID)=ObjInfo.pPolygon;
		*(nT+nID)=ObjInfo.nPolygon;
		*(pN+nID)=ObjInfo.pNormal;
		//To do: Update obj disp
	}
	
	for(long t=0; t<nObjects; t++)
	{
		CrestObjectInfo& TInfo = ObjectInfoV[t];
		if(TISSUE_TYPE!=TInfo.nObjectType) continue;
		long& nID=TInfo.nID;
		pVT=*(pV+nID);
		nVT=*(nV+nID);
		pTT=*(pT+nID);
		nTT=*(nT+nID);
		pNT=TInfo.pNormal;
		//To do: Update obj disp

		for(long n=0; n<nObjects; n++)
		{
			CrestObjectInfo& IInfo = ObjectInfoV[n];
			if(INSTRUMENT_TYPE!=IInfo.nObjectType) continue;
			long& nID=IInfo.nID;
			pVI=*(pV+nID);
			pTI=*(pT+nID);
			pNI=*(pN+nID);
			nVI=*(nV+nID);
			nTI=*(nT+nID);
			long &nCount1=TInfo.Response.nPPos[nID];
			tCRNode* pCRNodes1=TInfo.Response.PPos[nID];

			if(0>=nCount1) continue;
			vertex RO;
			MAKEVECTOR(RO, 0.0f, 0.0f, 0.0f);
			for(long i=0;i<nCount1;i++)
			{
				//long nID=pCRNodes1->id;
				VectorSum(&RO, pVT+(pCRNodes1+i)->id, &RO);
			}
			ScaleVector(&RO, 1.0f/nCount1, &RO);
			CDFloat fR2=0.001f;
			for(long i=0;i<nCount1;i++)
			{
				//long nID=pCRNodes1->id;
				fR2=max(VectorSquaredDistance(&RO, pVT+(pCRNodes1+i)->id), fR2);
			}
			fR2*=2.0f;
			printf("fR2= %lf\n", fR2);
			vertex NA;
			MAKEVECTOR(NA, 0.0, 0.0, 0.0);
			vertex NT;
			for(long i=0;i<nCount1;i++)
			{
				long nID=(pCRNodes1+i)->id;
				fR2=max(VectorSquaredDistance(&RO, pVT+(pCRNodes1+i)->id), fR2);
				MAKEVECTOR(NT, *(pNT+3*nID), *(pNT+3*nID+1), *(pNT+3*nID+2));
				VectorSum(&NA, &NT, &NA);
			}
			fR2*=2.0f;
			vertex Pole=NA;
			ScaleVector(&Pole, 1.0*sqrt(fR2)/VectorLength(&Pole), &Pole);
			VectorSum(&RO, &Pole, &Pole); // To do: Limit geo Pole within Instrument Pole (another ray cast).

			CDFloat t, t1, t0, u, r;
			t=u=r=-1.0f;
			vertex *pVert[3], *pOri; // 3 For Triangle
			vertex Dir, Dir1, Dir0;
			nCount1=0;
			long nOffset;
			const long nPrimitives = IInfo.nPolygonType;
			float fT=1.0f;
			t0=1000.0f;
			for(long i=0;i<nVI;i++)
			{
				if(nCount1>=RNODES_MAX) break;
				pOri=pVI+i;
				if( fR2<VectorSquaredDistance(&RO, pOri) ) continue;
				//(*(pGI+n))->GetDisplacement(j, &DI);// For V check...
					//pDT=&((pD->m_CurrentSys+i)->v);
					//if(0.0<DotProduct(pDT, &DI)) continue;
					//VectorDifference(&DI, pDT, &Dir);
				VectorDifference(&Pole, pOri, &Dir0);
				float* pFNI=pNI+i*3;
				vertex NI;
				MAKEVECTOR(NI, *pFNI, *(pFNI+1), *(pFNI+2));
				if(0.0<DotProduct(&NI, &NA)) continue; //if(0.0>DotProduct(&NI, &DI)) continue;
				t1=t0;
				long *pTri=NULL;
				long *pTri1=NULL;
				for(long j=0;j<nTT;j++)
				{
					if(nCount1>=RNODES_MAX) break;
					nOffset=j*nPrimitives;
					pTri=pTT+nOffset;
					*(pVert)=pVT+*(pTri);
					*(pVert+1)=pVT+*(pTri+1);
					*(pVert+2)=pVT+*(pTri+2);
					if( fR2<VectorSquaredDistance(*pVert, &RO) &&
						fR2<VectorSquaredDistance(*(pVert+1), &RO) &&
						fR2<VectorSquaredDistance(*(pVert+2), &RO) )
						continue;
					float* pFN=pNT+*(pTri)*3;//vertex &N=*(pNT+i); // For V check
					vertex N;
					MAKEVECTOR(N, *pFN, *(pFN+1), *(pNT+2));
					//VectorSum(&N, pNT+pTri->p2, &N);
					//VectorSum(&N, pNT+pTri->p3, &N);
					if(0.0>DotProduct(&N, &NA)) continue;//if(0.0<DotProduct(&N, &DI)) continue;
					//Dir.x=0.0-N;
					//Dir.y=0.0-(*(&N+1));              
					//Dir.z=0.0-(*(&N+2));
					//MAKEVECTOR(Dir, 0.0, -1.0, 0.0);
					if( 1 != intersect_triangle((CDFloat*)&(pOri->x), (CDFloat*)&Dir0,
					   (CDFloat*)(*pVert), (CDFloat*)(*(pVert+1)), (CDFloat*)(*(pVert+2)),
					   &t, &u, &r) ) continue;
					if(0.0f>=t||0.0f>u||0.0f>r) continue;
					if(t>t1) continue;		
					fT=min(VectorSquaredLength(&Dir0), fR2);
					//fT=min(fT, 2.0f*VectorSquaredLength(&DI));
					if(t*t>(fT)) continue;	
					Dir=Dir0;
					ScaleVector(&Dir, -1.0*t, &Dir);
					t1=t;
					Dir1=Dir;
					pTri1=pTri;
				}
				if(t1<t0){
					if(!TInfo.Response.IsNewCRNode(*pTri1, pCRNodes1, nCount1)) continue;
					VectorSum(&Dir1, pVT+*pTri1, &(pCRNodes1[nCount1].vector));
					//pCRNodes1[nCount1].vector=Dir1;
					pCRNodes1[nCount1].id=*pTri1;
					pCRNodes1[nCount1++].id_instrument=n; //pCRNodes1[nCount1++].id_iNode=(pTI+j)->p1;
					printf("t1= %lf\n", t1);
				}
			}
		}
	}
}

//void CColliResp::ItoTObject(CrestObjects *pObjects)
//{
//	const long nObjects=pObjects->m_nObjects;
//	CrestObjectInfoV& ObjectInfoV=pObjects->m_ObjectInfo;
//	if(nObjects!=ObjectInfoV.size()) return;
//	
//	vertex *pV[OBJECT_MAX], *pVT, *pVI; 
//	long *pT[OBJECT_MAX], *pTT, *pTI;
//	long nT[OBJECT_MAX], nV[OBJECT_MAX], nVT, nTT, nVI, nTI;
//	float* pNT; //vertex* pNT;
//	vertex *pDT, DI;
//	
//	for(long i=0; i<nObjects; i++)
//	{
//		CrestObjectInfo& ObjInfo = ObjectInfoV[i];
//		long& nID=ObjInfo.nID;
//		*(pV+nID)=(vertex*)ObjInfo.pVertex; // For V check
//		*(nV+nID)=ObjInfo.nVertex;
//		*(pT+nID)=ObjInfo.pPolygon;
//		*(nT+nID)=ObjInfo.nPolygon;
//	}
//	
//	for(long t=0; t<nObjects; t++)
//	{
//		CrestObjectInfo& TInfo = ObjectInfoV[t];
//		if(TISSUE_TYPE!=TInfo.nObjectType) continue;
//		long& nID=TInfo.nID;
//		pVT=*(pV+nID);
//		nVT=*(nV+nID);
//		pTT=*(pT+nID);
//		nTT=*(nT+nID);
//		pNT=TInfo.pNormal;
//
//		for(long n=0; n<nObjects; n++)
//		{
//			CrestObjectInfo& IInfo = ObjectInfoV[n];
//			if(INSTRUMENT_TYPE!=IInfo.nObjectType) continue;
//			long& nID=IInfo.nID;
//			pVI=*(pV+nID);
//			pTI=*(pT+nID);
//			nVI=*(nV+nID);
//			nTI=*(nT+nID);
//			long &nCount1=TInfo.Response.nPPos[nID];
//			tCRNode* pCRNodes1=TInfo.Response.PPos[nID];
//
//			if(0>=nCount1) continue;
//			vertex RO;
//			MAKEVECTOR(RO, 0.0f, 0.0f, 0.0f);
//			for(long i=0;i<nCount1;i++)
//			{
//				//long nID=pCRNodes1->id;
//				VectorSum(&RO, pVT+(pCRNodes1+i)->id, &RO);
//			}
//			ScaleVector(&RO, 1.0f/nCount1, &RO);
//			CDFloat fR2=0.001f;
//			for(long i=0;i<nCount1;i++)
//			{
//				//long nID=pCRNodes1->id;
//				fR2=max(VectorSquaredDistance(&RO, pVT+(pCRNodes1+i)->id), fR2);
//			}
//			//fR2*=2.0f;
//			printf("fR2= %lf\n", fR2);
//			
//			CDFloat t, t1, t0, u, r;
//			t=u=r=-1.0f;
//			vertex *pVert[3], *pOri; // 3 For Triangle
//			vertex Dir, Dir1;
//			nCount1=0;
//			long nOffset;
//			const long nPrimitives = IInfo.nPolygonType;
//			t0=1000.0f;
//			for(long i=0;i<nVT;i++)
//			{
//				if(nCount1>=RNODES_MAX) break;
//				pOri=pVT+i;
//				if( fR2<VectorSquaredDistance(&RO, pOri) || !TInfo.Response.IsNewCRNode(i, pCRNodes1, nCount1) ) continue;
//				float* pN=(pNT+i*3);//vertex &N=*(pNT+i); // For V check
//				t1=t0;
//				for(long j=0;j<nTI;j++)
//				{
//					if(nCount1>=RNODES_MAX) break;
//					nOffset=j*nPrimitives;
//					*(pVert)=pVI+*(pTI+nOffset++);
//					*(pVert+1)=pVI+*(pTI+nOffset++);
//					*(pVert+2)=pVI+*(pTI+nOffset);
//					if( fR2<VectorSquaredDistance(*pVert, &RO) &&
//						fR2<VectorSquaredDistance(*(pVert+1), &RO) &&
//						fR2<VectorSquaredDistance(*(pVert+2), &RO) )
//						continue;
//					//(*(pGI+n))->GetDisplacement(j, &DI);// For V check...
//					//pDT=&((pD->m_CurrentSys+i)->v);
//					//if(0.0<DotProduct(pDT, &DI)) continue;
//					//VectorDifference(&DI, pDT, &Dir);
//					Dir.x=0.0-(*(pN));
//					Dir.y=0.0-(*(pN+1));              
//					Dir.z=0.0-(*(pN+2));
//					//MAKEVECTOR(Dir, 0.0, -1.0, 0.0);
//					if( 1 != intersect_triangle((CDFloat*)&(pOri->x), (CDFloat*)&Dir,
//					   (CDFloat*)(*pVert), (CDFloat*)(*(pVert+1)), (CDFloat*)(*(pVert+2)),
//					   &t, &u, &r) ) continue;
//					//intersect_triangle((CDFloat*)&(pOri->x), (CDFloat*)&Dir,
//					//	(CDFloat*)(*pVert), (CDFloat*)(*(pVert+1)), (CDFloat*)(*(pVert+2)),
//					//	&t, &u, &r);
//					//t=min(t, sqrt(fR2));
//					//if(0.0f>=t||-.002f>u||-.002f>r) continue;
//					if(0.0f>=t||-0.0f>u||-0.0f>r) continue;
//					if(t>t1) continue;		
//					//Norm(&Dir);
//					//ScaleVector(&Dir, 0.1, &Dir); //ScaleVector(&Dir, sqrt(fR2), &Dir); 
//					ScaleVector(&Dir, t, &Dir); //ScaleVector(&Dir, sqrt(fR2), &Dir);
//					VectorSum(&Dir, pOri, &Dir);
//					t1=t;
//					Dir1=Dir;
//				}
//				if(t1<t0){
//					pCRNodes1[nCount1].vector=Dir1;
//					pCRNodes1[nCount1].id=i;
//					pCRNodes1[nCount1++].id_instrument=n; //pCRNodes1[nCount1++].id_iNode=(pTI+j)->p1;
//					printf("t1= %lf\n", t1);
//				}
//			}
//		}
//	}
//}

