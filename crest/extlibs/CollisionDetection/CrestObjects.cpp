//#include "StdAfx.h"
#include "CrestObjects.h"

CrestObjects::CrestObjects()
{
	m_pObjectInfo=NULL;
	ResetCrestObjects();
}

CrestObjects::~CrestObjects()
{
	ResetCrestObjects();
}

void CrestObjects::ResetCrestObjects()
{
	if(m_pObjectInfo!=NULL) delete m_pObjectInfo;
	m_nObjects=0;
	m_pObjectInfo=NULL;
}

void CrestObjects::ClearCRNodes(bool bC, bool bR)
{
	tCRNode* pCRNode;
	for(long k=0;k<m_nObjects;k++)
	{
		ResponseData *pResponse=m_pObjectInfo[k].pResponse;
		if(NULL==pResponse) continue;
		for(long i=0;i<INSTRUMENT_MAX;i++)
		{
			if(bC || GetObjectInfo(i)->pIInfo->nGraspStatus==0)
			{	
				pResponse->nCPos[i]=0;
			}

			if(bR)
			{
				pResponse->nPPos=0;
				//for(i=0;i<RNODES_MAX;i++) m_RNode[i].pVector=NULL;
				pResponse->nRPos=pResponse->nRVelocity=pResponse->nRForce=0;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
CrestObjectInfo* CrestObjects::GetObjectInfo(long nID)
{
	CrestObjectInfo *pObjectInfo = m_pObjectInfo;
	for(long i=0;i<m_nObjects;i++)
	{
		if(nID==(pObjectInfo++)->nID) return pObjectInfo;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
ResponseData::ResponseData()
{
	long nRPos=nPPos=nRVelocity=nRForce=0;
	for(int i=0;i<INSTRUMENT_MAX;i++)
	{
		nCPos[i]=0;
	}
}

ResponseData::~ResponseData(){}

bool ResponseData::IsNewCRNode(long n, bool bRNode)
{
	long nTotal;
	tCRNode* pCRNode;

	if(bRNode)
	{
		pCRNode=RForce;
		nTotal=nRForce;
		for(long j=0; j<nTotal; j++)
		{
			if(n==pCRNode[j].id) return false;
		}
	}
	else{
		for(long i=0; i<INSTRUMENT_MAX; i++)
		{
			pCRNode=CPos[i];
			nTotal=nCPos[i];
			for(long j=0; j<nTotal; j++)
			{
				if(n==pCRNode[j].id) return false;
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool CrestInstrumentInfo::IsMedial(long nID)
{
	return false;
}