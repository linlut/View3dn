/////////////////////////////////////////////////////////////////////////////
/// Collision Detection and Response
/// CREST 2008
/////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __CCOLLIRESP_H__
#define __CCOLLIRESP_H__

#include "Defines.h"
#include "CrestObjects.h"

class CPartiSys;
/////////////////////////////////////////////////////////////////////////////
class CColliResp
{
public:
	CColliResp(void);
	~CColliResp(void);

	void CollisionResponse(long nCollisionType, long nWhichI, long nWhichR, long nWhichT, tTrianglePhy tp[]);
	static void ItoTObject(CrestObjects *pObjects);

};

#endif // of #ifndef __CCOLLIRESP_H__