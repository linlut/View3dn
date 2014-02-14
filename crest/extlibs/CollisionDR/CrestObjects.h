/////////////////////////////////////////////////////////////////////////////
/// Collision Detection and Response
/// CREST 2008
/////////////////////////////////////////////////////////////////////////////
#ifndef __CRESTOBJECTS_H__
#define __CRESTOBJECTS_H__


#include <vector>
#include "Defines.h"

#pragma once
#pragma warning (disable:4786)

using namespace std;

/////////////////////////////////////////////////////////////////////////
#ifndef CRNodeStructures
#define CRNodeStructures
	struct tCRNode
	{
		long id;			///< index of this node
		long id_instrument;	///< object id of instrument grasping this node
		long id_iNode;		///< node id of instrument grasping this node
		//vertex *pVector;	///< pointing to changed position
		vertex vector;		///< amount of change of velocity, force or positioin
	};
#endif
#ifndef ResponseDataClass
#define ResponseDataClass
	class ResponseData
	{
	public:
		tCRNode CPos[INSTRUMENT_MAX][CNODES_MAX];	///< pointing to changed position due to grasping instrument
		tCRNode PPos[INSTRUMENT_MAX][RNODES_MAX];	///< changed position due to pushing instrument
		long nCPos[INSTRUMENT_MAX]; 
		long nPPos[INSTRUMENT_MAX];
		/// no feedback to instrument:
		tCRNode RPos[RNODES_MAX];		///< changed position
		tCRNode RVelocity[RNODES_MAX];	///< changed velocity
		tCRNode RForce[RNODES_MAX];		///< changed position
		long nRPos;
		long nRVelocity;
		long nRForce; 
	public:
		ResponseData();
		~ResponseData();
		bool IsNewCRNode(long n, bool bRNode);
		bool IsNewCRNode(long n, tCRNode* pCRNode, long nExisting);
	};
#endif

#ifndef CrestInstrumentInfoClass
#define CrestInstrumentInfoClass
	class CrestInstrumentInfo
	{
	public:
		long id;
		CDFloat fJawRadian;
		CDFloat fJawRadianOld;
		int nGraspStatus;///< three status 1/-1/0 for grasped/closed or holding/releasing
		bool *pMID;
	public:
		CrestInstrumentInfo();
		~CrestInstrumentInfo();
		bool IsMedial(long nID);
		int GetStatus();
	};
#endif

#ifndef CrestObjectInfoClass
#define CrestObjectInfoClass
	class CrestObjectInfo
	{
	public:
		CDFloat *pVertex;
		float *pNormal;//CDFloat *pNormal;
		long nVertex;

		long *pElement;
		long nElement;
		long nElementType; /// Tet: 4; Hex: 8

		long *pPolygon; /// Triangle
		long nPolygon;
		long nPolygonType; /// 3 for Triangle

		long nID; /// Object ID
		long nObjectType; /// Instrument

		ResponseData Response; /// Double buffer it if CD/Def in two threads
		CrestInstrumentInfo IInfo;
	public:
		CrestObjectInfo(long nObjectType=TISSUE_TYPE);
		~CrestObjectInfo();
	};
#endif

typedef vector<CrestObjectInfo> CrestObjectInfoV;
/////////////////////////////////////////////////////////////////////////

class CrestObjects
{
public:
	CrestObjectInfoV m_ObjectInfo;
	long m_nObjects;
 
public:
	CrestObjects();
	virtual ~CrestObjects();
	void ResetCrestObjects();
	CrestObjectInfo* GetObjectInfo(long nID);
	bool AddObjectInfo(CrestObjectInfo ObjInfo);
	bool DeleteObjectInfo(long nID);
	void ClearObjectInfo();
	void ClearCRNodes(BOOL bC, BOOL bR);
};

#endif /// END OF THE .H FILE

/*
Object ID: in Defines.h

/// Maximum objects in the scene...
#define TISSUE_MAX 64	
#define INSTRUMENT_MAX 8 ///< no less than 4 for hardware config
#define RIGID_MAX 64
#define CNODES_MAX 36	///< Maximum control nodes.
#define RNODES_MAX 1000	///< Maximum response nodes.
//#define GNODES_MAX 36	///< Maximum grasping nodes.
#define VISUAL_MAX 36

//Sequence:
//Instruments, rigid, tissue, visual


/// TYPE OF OBJECTS IN THE SCENE
#define INSTRUMENT_TYPE 0
#define RIGID_TYPE 1
#define TISSUE_TYPE 2 
#define PARTI_TYPE 3 ///< particle system simulation
#define VISUAL_TYPE 4 ///< Only for graphic rendering
*/

