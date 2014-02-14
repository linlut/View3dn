//////////////////////////////////////////////////////////////////////////////
/// MedVR Simulation
/// S.Y.H.
/// Virtual Environment Lab
/// Biomedical Engineering
/// Electrical Engineering
/// UTSW, UTA
/////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __CCOLLIRESP_H__
#define __CCOLLIRESP_H__

#include "Defines.h"

class CPartiSys;
/////////////////////////////////////////////////////////////////////////////
class CColliResp
{
public:
	CColliResp(void);
	~CColliResp(void);

	void CollisionResponse(long nCollisionType, long nWhichI, long nWhichR, long nWhichT, tTrianglePhy tp[]);


};

#endif // of #ifndef __CCOLLIRESP_H__