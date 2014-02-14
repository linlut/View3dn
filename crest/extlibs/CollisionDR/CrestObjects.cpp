/////////////////////////////////////////////////////////////////////////////
/// Collision Detection and Response
/// CREST 2008
/////////////////////////////////////////////////////////////////////////////
//#include "StdAfx.h"
#include "CrestObjects.h"


CrestObjects::CrestObjects()
{
	ResetCrestObjects();
}

CrestObjects::~CrestObjects()
{
	ResetCrestObjects();
}

void CrestObjects::ResetCrestObjects()
{
	ClearObjectInfo();
	m_nObjects=0;
}

void CrestObjects::ClearCRNodes(BOOL bC, BOOL bR)
{
	tCRNode* pCRNode;
	for(long k=0;k<m_nObjects;k++)
	{
		ResponseData* pResponse=&(m_ObjectInfo[k].Response);
		if(TISSUE_TYPE!=m_ObjectInfo[k].nObjectType) continue;
		for(long i=0;i<INSTRUMENT_MAX;i++)
		{
			if(bC || GetObjectInfo(i)->IInfo.GetStatus()==0)
			{	
				pResponse->nCPos[i]=0; // Instrument ID starts from zero to match with array index
			}

			if(bR)
			{
				for(i=0;i<INSTRUMENT_MAX;i++)
				{
					pResponse->nPPos[i]=0;
				}
				//for(i=0;i<RNODES_MAX;i++) m_RNode[i].pVector=NULL;
				pResponse->nRPos=pResponse->nRVelocity=pResponse->nRForce=0;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
CrestObjectInfo* CrestObjects::GetObjectInfo(long nID)
{
	m_nObjects=m_ObjectInfo.size();
	for(long i=0;i<m_nObjects;i++)
	{
		if(nID==m_ObjectInfo[i].nID) return &m_ObjectInfo[i];
	}
	return NULL;
}

bool CrestObjects::AddObjectInfo(CrestObjectInfo ObjInfo)
{
	if(m_nObjects>OBJECT_MAX || m_ObjectInfo.size()>OBJECT_MAX) return false;

	m_ObjectInfo.push_back(ObjInfo);
	if(m_ObjectInfo.size()==m_nObjects+1)
	{
		m_nObjects++;
		return true;
	}
	else return false;
}

bool CrestObjects::DeleteObjectInfo(long nID)
{
	CrestObjectInfoV::iterator Iter=m_ObjectInfo.begin();
	if(m_ObjectInfo.size()==0) return false;
	while(Iter!=m_ObjectInfo.end())
	{
		if(nID==Iter->nID)
		{
			m_ObjectInfo.erase(Iter);
			m_nObjects=m_ObjectInfo.size();
			return true;
		}
		Iter++;
	}
	return false;
}

void CrestObjects::ClearObjectInfo()
{
	m_ObjectInfo.clear();
	m_nObjects=m_ObjectInfo.size();
}

///////////////////////////////////////////////////////////////////////////////
ResponseData::ResponseData()
{
	nRPos=nRVelocity=nRForce=0;
	for(int i=0;i<INSTRUMENT_MAX;i++)
	{
		nCPos[i]=nPPos[i]=0;
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
			if(n==pCRNode[j].id) return FALSE;
		}
	}
	else{
		for(long i=0; i<INSTRUMENT_MAX; i++)
		{
			pCRNode=CPos[i];
			nTotal=nCPos[i];
			for(long j=0; j<nTotal; j++)
			{
				if(n==pCRNode[j].id) return FALSE;
			}
		}
	}

	return TRUE;
}

bool ResponseData::IsNewCRNode(long n, tCRNode* pCRNode, long nExisting) // RNodes
{
	for(long j=0; j<nExisting; j++)
	{
		if(n==pCRNode[j].id)
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
CrestInstrumentInfo::CrestInstrumentInfo()
{
	pMID=NULL;
	fJawRadian=0.0f;
	fJawRadianOld=0.0f;
	nGraspStatus=1;
}
CrestInstrumentInfo::~CrestInstrumentInfo()
{}

bool CrestInstrumentInfo::IsMedial(long nID)
{
	if(pMID==NULL) return false;
	else return pMID[nID];
}

int CrestInstrumentInfo::GetStatus()
{
		if(fJawRadian<0.176)
		{
			if(fJawRadian<fJawRadianOld && fJawRadian>0.056) nGraspStatus=1; // grasping
			else nGraspStatus=-1; // holding
		}
		else nGraspStatus=0; // releasing	return ;
		return nGraspStatus;
}
///////////////////////////////////////////////////////////////////////////////
CrestObjectInfo::CrestObjectInfo(long nObjectType): Response(), IInfo()
{
	CrestObjectInfo::nObjectType=nObjectType;
	//if(TISSUE_TYPE==nObjectType)
	//{
	//	pResponse=new ResponseData;
	//	pIInfo=NULL;
	//}
	//else if(INSTRUMENT_TYPE==nObjectType)
	//{
	//	pIInfo=new CrestInstrumentInfo;
	//	pResponse=NULL;
	//}

	pVertex=NULL;
	pNormal=NULL;
	nVertex=0;

	pElement=NULL;
	nElement=0;
	nElementType=-1; /// Tet: 4; Hex: 8

	pPolygon=NULL; /// Triangle
	nPolygon=0;

	nID=-1; /// Object ID

}

CrestObjectInfo::~CrestObjectInfo()
{
	//if(pResponse!=NULL)	delete pResponse;
	//else if(pIInfo!=NULL) delete pIInfo;
}