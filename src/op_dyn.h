/*------------------------------------------------------ 
//
//op_dyn.h
//
//------------------------------------------------------*/

#ifndef __INC_CIA3D_OPTION_DYN_H__
#define __INC_CIA3D_OPTION_DYN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include <sysconf.h>
#include <vectorall.h>
#include <qdom.h>

#include "nametableitem.h"


class CCia3dOptionDyn
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


private:



public:

	CCia3dOptionDyn()
	{
		m_nIDFormulation = 1;
		m_nTimeIntegrationAlg =1;	
	}

	~CCia3dOptionDyn()
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



extern void PrepareXmlSymbolTable(CCia3dOptionDyn &cio, CNameTableItem table[]);

#endif