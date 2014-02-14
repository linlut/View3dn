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
	
	vertex *pT[OBJECT_MAX], *pV[OBJECT_MAX], *pVT, *pTT, *pVI, *pTI;
	long nT[OBJECT_MAX], nV[OBJECT_MAX], nVT, nTT, nVI, nTI;
	vertex* pNT;
	vertex *pDT, DI;
	
	for(long i=0; i<nObjects; i++)
	{
		CrestObjectInfo& ObjInfo = ObjectInfoV[i];
		long& nID=ObjInfo.nID;
		*(pV+nID)=(vertex*)ObjInfo.pVertex; // For V check
		*(nV+nID)=ObjInfo.nVertex;
		*(pT+nID)=(vertex*)ObjInfo.pPolygon;
		*(nT+nID)=ObjInfo.nPolygon;
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
		pNT=(vertex*)TInfo.pNormal;

		for(long n=0; n<nObjects; n++)
		{
			CrestObjectInfo& IInfo = ObjectInfoV[n];
			if(INSTRUMENT_TYPE!=IInfo.nObjectType) continue;
			long& nID=IInfo.nID;
			pVI=*(pV+nID);
			pTI=*(pT+nID);
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
			CDFloat fR2=5.0f;
			for(long i=0;i<nCount1;i++)
			{
				//long nID=pCRNodes1->id;
				fR2=max(VectorSquaredDistance(&RO, pVT+(pCRNodes1+i)->id), fR2);
			}
			//fR2*=2.0f;
			
			CDFloat t, u, r;
			t=u=r=-1.0f;
			vertex *pVert[3], *pOri;
			vertex Dir;
			nCount1=0;
			for(long i=0;i<nVT;i++)
			{
				if(nCount1>=RNODES_MAX) break;
				pOri=pVT+i;
				if( fR2<VectorSquaredDistance(&RO, pOri) || !TInfo.Response.IsNewCRNode(i, pCRNodes1, nCount1) ) continue;
				vertex &N=*(pNT+i); // For V check
				for(long j=0;j<nTI;j++)
				{
					if(nCount1>=RNODES_MAX) break;
					*(pVert)=pVI+(pTI+j)->p1;
					*(pVert+1)=pVI+(pTI+j)->p2;
					*(pVert+2)=pVI+(pTI+j)->p3;
					if( fR2<VectorSquaredDistance(*pVert, &RO) &&
						fR2<VectorSquaredDistance(*(pVert+1), &RO) &&
						fR2<VectorSquaredDistance(*(pVert+2), &RO) )
						continue;
					//(*(pGI+n))->GetDisplacement(j, &DI);// For V check...
					//pDT=&((pD->m_CurrentSys+i)->v);
					//if(0.0<DotProduct(pDT, &DI)) continue;
					//VectorDifference(&DI, pDT, &Dir);
					Dir.x=0.0f-N.x;
					Dir.y=0.0f-N.y;
					Dir.z=0.0f-N.z;
					if( 1 != intersect_triangle((CDFloat*)&(pOri->x), (CDFloat*)&Dir,
					   (CDFloat*)(*pVert), (CDFloat*)(*(pVert+1)), (CDFloat*)(*(pVert+2)),
					   &t, &u, &r) ) continue;
					//intersect_triangle((CDFloat*)&(pOri->x), (CDFloat*)&Dir,
					//	(CDFloat*)(*pVert), (CDFloat*)(*(pVert+1)), (CDFloat*)(*(pVert+2)),
					//	&t, &u, &r);
					//t=min(t, sqrt(fR2));
					if(0.0f>=t||-5.5f>u||-5.5f>r) continue;
					ScaleVector(&Dir, t, &Dir);
					//ScaleVector(&Dir, sqrt(fR2), &Dir);
					VectorSum(&Dir, pOri, &Dir);
					pCRNodes1[nCount1].vector=Dir;
					pCRNodes1[nCount1].id=i;
					pCRNodes1[nCount1++].id_instrument=n; //pCRNodes1[nCount1++].id_iNode=(pTI+j)->p1;
				}
			}
		}
	}
}

