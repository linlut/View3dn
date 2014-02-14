/*------------------------------------------------------ 
//
//op_imp.h
//
//------------------------------------------------------*/

#ifndef __INC_CIA3D_OPTION_IMP_H__
#define __INC_CIA3D_OPTION_IMP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <sysconf.h>
#include <vectorall.h>
#include <qdom.h>

#include "nametableitem.h"


class CCia3dOptionImp
{
public:

	int m_nIDFormulation;
	// in {1, 2}
	//1: Total Lagrangian formulation (TLF),
	//2: Arbitrary reference confuration (ARC) Lagrangian formulation (ARCLF), and pure explicit FDCD

	int m_nTimeIntegrationAlg;
	//1 for Newmark average acceleration method with full Newton-Raphson iteration.
	//2 for Newmark average acceleration method with modified Newton-Raphson iteration
	//3 for forward incremental displacement central difference (FIDCD method
	//4 DO NOT USE (OLD EL method).
	//5 for forward displacement second-order accurate nonlinearly explicit Lstable (FDEL) method

	float m_fG_MAX;			//auto search radius
	float m_fV_DOT_N_MIN;	//dot product (angle)
	//G_MAX in [0, +INF]
	//V_DOT_N_MIN in [-1.0, 1.0]
	//1.6 maximum global search radii
	//-0.1 V*N-- dot product between the object initial velocity and the master segment normal direction, the minimum search direction

	int m_nContactSearch;
	//in {1, 2}
	//1: only search contact between bodies
	//2: include self-contact search

	int m_nLSWITCH;		//family switch
	//in {1, 2, 3, 4, 5}
	//1 no switch
	//2 stupid switch: given time step, auto select FDCD or FDEL
	//3 contact switch: auto select FDCD with critical time step tcritical for contact and select FDEL with time step FACTOR*tcritical for dynamic response
	//4 element based smart switch: in element based formulation auto select FDCD with critical time step tcritical and FDEL with time step 
	//FACTOR*tcritical depdent on the current occuring physics such as high velocity impact,
    //strain rate, damage, etc.
	//5 smart switch: auto select FDCD with critical time step tcritical and FDEL
	//with time step FACTOR*tcritical depdent on the current occuring physics
	//such as high velocity impact, strain rate, damage, etc.

	int m_nLSOLV;    //valid only when m_nTimeIntegrationAlg is 5
	//LSOLV2 {1, 2, 3}
	//1 FDEL family: conditional stable FDCD
	//2 FDEL family: weak stable
	//3 FDEL family: strong stable


	char m_cAdaptive; //valid only when m_nTimeIntegrationAlg is 5
	//in {y, n}
	//y using adaptive time steping, automatic calculate critical time step
	//n using the fixed time step provided in c3d1.in

	
	float m_fFactor;	//Requires m_nTimeIntegrationAlg=5 AND m_cAdaptive=y
	//FACTOR in [0, +INF]
	//4.0 the factor of time step for EL in the unit of CD critical time step

	
	float m_fGAP_SW; //IF m_nLSWITCH=3
	//GAP SW2 [0,+INF]
	//0.0001 when minimum gap distant greater than GAP SW, use FDEL, otherwise use FDCD

private:


public:

	CCia3dOptionImp()
	{
		m_nIDFormulation = 1;
		m_nTimeIntegrationAlg =5;	

		m_fG_MAX = 0.02f;	
		m_fV_DOT_N_MIN = -0.0001f;	//dot product (angle)

		m_nContactSearch = 2;
		//1: only search contact between bodies
		//2: include self-contact search

		m_nLSWITCH = 2;		//family switch
		//1 no switch
		//2 stupid switch: given time step, auto select FDCD or FDEL
		//3 contact switch: auto select FDCD with critical time step tcritical for contact and select FDEL with time step FACTOR*tcritical for dynamic response
		//4 element based smart switch: in element based formulation auto select FDCD with critical time step tcritical and FDEL with time step 
		//FACTOR*tcritical depdent on the current occuring physics such as high velocity impact,
		//strain rate, damage, etc.
		//5 smart switch: auto select FDCD with critical time step tcritical and FDEL
		//with time step FACTOR*tcritical depdent on the current occuring physics
		//such as high velocity impact, strain rate, damage, etc.

		m_nLSOLV=1;    //valid only when m_nTimeIntegrationAlg is 5
		//1 FDEL family: conditional stable FDCD
		//2 FDEL family: weak stable
		//3 FDEL family: strong stable


		m_cAdaptive='y'; //valid only when m_nTimeIntegrationAlg is 5
		//y using adaptive time steping, automatic calculate critical time step
		//n using the fixed time step provided in c3d1.in
	
		m_fFactor =5;	//Requires m_nTimeIntegrationAlg=5 AND m_cAdaptive=y
		//FACTOR in [0, +INF]
		//4.0 the factor of time step for EL in the unit of CD critical time step

		m_fGAP_SW = 0.000001f; //IF m_nLSWITCH=3
		//GAP SW2 [0,+INF]
		//0.0001 when minimum gap distant greater than GAP SW, use FDEL, otherwise use FDCD
	}

	~CCia3dOptionImp()
	{
		
	}

	bool loadFile(const char *fname);

	bool saveFile(const char *fname);

	bool loadXMLFile(const char *fname);

	void saveXMLFile(const char *fname);

	//Get a copy of the single object, where the vertices and mesh is indexed independently
	//void getSingleObject(const int objid, Vector3f *& pVertex, int &nVertex, void* &pElement, int &nElement);

	QDomDocument *createDomDocument(void);

	void convertFromDomDocument(QDomDocument *pdom);

};



extern void PrepareXmlSymbolTable(CCia3dOptionImp &cio, CNameTableItem table[]);

#endif