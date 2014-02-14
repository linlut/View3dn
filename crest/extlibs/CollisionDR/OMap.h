/////////////////////////////////////////////////////////////////////////////
/// Collision Detection
/// CREST 2008
/////////////////////////////////////////////////////////////////////////////
#ifndef __OMap_H__
#define __OMap_H__

#pragma once
#pragma warning (disable:4786)


#define LONG_ID unsigned __int64 //DWORD // ///< distinct collision index for cell_wise redundancy check

#define RASTER_SIZE 1024 ///< Maximum number of cell indices reserved per primitive rasterization

//C:/Program Files/Microsoft Visual Studio .NET/Vc7/include/
//C:/USERS/syh/sgiSTL/
//#include "C:/Program Files/Microsoft Visual Studio .NET/Vc7/include/list"
//#include "C:/Program Files/Microsoft Visual Studio .NET/Vc7/include/vector"
//#include "C:/Program Files/Microsoft Visual Studio .NET/Vc7/include/map"
//#include "C:/Program Files/Microsoft Visual Studio .NET/Vc7/include/hash_map"
//#include "C:/Program Files/Microsoft Visual Studio .NET/Vc7/include/hash_set"
//#include "C:/Program Files/Microsoft Visual Studio .NET/Vc7/include/set"

//#include "C:/USERS/syh/sgiSTL/list"
//#include "C:/USERS/syh/sgiSTL/vector"
//#include "C:/USERS/syh/sgiSTL/map"
//#include "C:/USERS/syh/sgiSTL/hash_map"
//#include "C:/USERS/syh/sgiSTL/hash_set"
//#include "C:/USERS/syh/sgiSTL/set"

//#include "list"
//#include "vector"
//#include "map"
//#include "hash_map"
//#include "hash_set"
//#include "set"

#include <list>
#include <vector>
#include <map>
#include <hash_map>
#include <hash_set>
#include <set>

using namespace std;

#include "Defines.h"

class CDeformable;
class CPartiSys;
//class COcclusionTest;
class CrestObjects;

/////////////////////////////////////////////////////////////////////////////

class COMap
{
	/////////////////////////////////////////////////////////////////////////
	#ifndef OMapStructures
	#define OMapStructures

	/*// Link of index of vertices or voxel primitives
	struct tVidLink	
	{
		long id;
		tVidLink* pNext;
	};
	// Link of object
	struct tObjLink	
	{
		long id;
		tVidLink* pVidHead;
		tObjLink* pNext;
	};
	// Link of occupancy map
	struct tOMapLink
	{
		DWORD id;
		long count;
		tObjLink* pObjHead;
		tOMapLink* pNext;
	};*/

	/*#ifdef _DEBUG
	#define tVidList A
	#define tObjMap B
	#define tOMap C
	#endif*/

	// tVidList group is replaced by tVidVector group:
	//typedef list<long> tVidList;
	//typedef map<long, tVidList> tObjMap;
	//typedef map<DWORD, tObjMap> tOMap;
	//typedef pair<long, long> tCo;
	//typedef map<DWORD, tCo> tCounterMap;

	// tVidVector group
	//typedef vector<long> tVidVector;
	//typedef hash_map<long, tVidVector> tObjMap;
	//typedef hash_map<DWORD, tObjMap> tOMap;
	//typedef pair<long, long> tCo;
	//typedef hash_map<DWORD, tCo> tCounterMap;

	// tVidVector group
	//typedef list<long> tVidList;
	//typedef hash_map<long, tVidList> tObjMap;
	//typedef list<tObjMap> tObjMapList;
	//typedef hash_map<DWORD, tObjMapList::iterator> tOMap;
	//typedef pair<long, long> tCo;
	//typedef hash_map<DWORD, tCo> tCounterMap;

	//typedef list<long> tVidList;
	//typedef hash_map<long, tVidList> tObjMap;
	//typedef hash_map<DWORD, tObjMap> tOMap;
	//typedef pair<long, long> tCo;
	//typedef hash_map<DWORD, tCo> tCounterMap;

	/// @name OHC DATA STRUCTURE
	//@{ 
	typedef vector<long> tVidVector; ///< Primitive ID, primitive layer data container
	typedef map<long, tVidVector> tObjMap; ///< Object layer, STL BST(binary search tree)(red-black)
	//typedef map<DWORD, tObjMap> tCellMap; ///< Cell layer as a BST
	//typedef vector<tCellMap> tOMap;
	typedef map<DWORD, tObjMap> tOMap; ///< Cell layer, each entry as a BST
	//@}
	
	/// @name Ojbect Counting Table
	//@{	
	typedef pair<long, long> tCo; ///< Object counter and the last object id
	typedef stdext::hash_map<DWORD, tCo> tCounterMap; ///< Cell id and counter pair
	//@}

	/// @name Cell_wise Redundancy Filter
	//@{	
	//typedef pair<DWORD, DWORD> LONG_ID;
	typedef set<LONG_ID> tCollisionSet; ///< cell_wise redundancy check
	//@}

	/// @name Particle system detection
	//@{	
	typedef vector<long> tCidVector;
	typedef map<long, tCidVector> tPrimCell;
	typedef stdext::hash_map<long, tPrimCell> tObjPrimCell;
	//@}

	#endif
	/////////////////////////////////////////////////////////////////////////

// Construction
public:
	COMap();
	virtual ~COMap();

// Attributes
public:
	//COcclusionTest* m_pOccTest;
	/// output counters: All intersection tests, Distinct tests, positive tests, detected distinct collisions
	long m_nAPairs, m_nTPairs, m_nPPairs, m_nDPairs; 
protected:
	//tOMapLink* m_pOMap;
	DWORD m_idTCell[RASTER_SIZE]; ///< Cell indices buffer for each primitive rasterization
	long m_nCountTCell; ///< valid size of the previous buffer
	long m_dwR; ///< Cell size 
		//DWORD m_dwR; ///< Cell size; sign error if raw data are not converted to positive
	tTrianglePhy m_tp[2]; ///< Data carriers between CD/CR

	//tOMap m_OMap[1019];//, m_OMap0;
	tObjPrimCell m_opc; ///< For particle system

private:

// Operations
public:
	int LookUpMap(); ///< Main CD function call
	int LookUpMap(CrestObjects *pObjects); ///< Main CD call for simpler interface
protected:
	inline void DeleteOMap(); ///< Empty OHC data
	void FillOMapT(tOMap* m_OMap, long nObject, vertex* v, long nV, vertex* t, long nT); ///< OHC data Construction
	void FillOMapT(tOMap* m_OMap, long nObject, CDFloat* v, long nV, long* t, long nT); ///< OHC data Construction for simpler interface
	void FillOMapParti(tOMap* m_OMap, CPartiSys& partisys);
	void FillOMapParti(tOMap* m_OMap);
	void FillCounters(tCounterMap& counterMap, long nObject, vertex* v, long nV, vertex* t, long nT); ///< Update object counting table
	void FillCountersParti(tCounterMap& counterMap, CPartiSys& partisys);
	inline void FillCell(tOMap* m_OMap, long nObject, long nVid, DWORD dwCell); ///< Instantiate a cell in OHC
	int DetectCRNode(tOMap* m_OMap); ///< Derive each pair from OHC for intersection test, and call CR
	int MarchingCells(tOMap* m_OMap, CrestObjects *pObjects); ///< Derive each pair from OHC for intersection test, and call CR
	int DetectCRParti(tOMap* m_OMap);
	void MeasureCP(tOMap* m_OMap); ///< Measure collisions and intersection tests
	void MeasureCPParti(tOMap* m_OMap);
	inline void CallResponse(long id[], long nType[], CDeformable* pD[]);
	inline void CallResponse(long id[], long nType[], CrestObjects *pObjects);
	//int DetectCRNodeOcc(tOMap* m_OMap);
	//inline static bool IsNewCRNode(long n, bool bRNode, CDeformable* pD); 
	//inline static bool IsNewCRNode(long n, tCRNode* pCRNode, long nExisting); // RNodes
	inline void RasterAABB(vertex* v, long nV, DWORD* pCid, long& nCid, const long nCidLimit); ///< AABB bounded rasterization
	inline DWORD Coor2dw(vertex& v); ///< coordinates hashing
	inline DWORD Hash(DWORD dwCell); 
	inline DWORD PairHash(LONG_ID nPair);
	inline void PairIndex(LONG_ID& nPair, long nObj1, long nObj2, long nV1, long nV2);
	inline bool CellwiseFilter(tCollisionSet* cset, long& nId0, long& nId1, long& nVid0, long& nVid1);


private:

};

#endif