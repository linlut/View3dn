/*------------------------------------------------------ 
//
//op_output.h
//
//------------------------------------------------------*/

#ifndef __INC_CIA3D_OPTION_OUTPUT_H__
#define __INC_CIA3D_OPTION_OUTPUT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include <sysconf.h>
#include <vectorall.h>
#include <qdom.h>

#include "nametableitem.h"


class CCia3dOptionOutput
{
public:
	char m_cPRINT;
	char m_cGDATA;
	char m_cDISPL;
	char m_cVELOC;
	char m_cSTRES;
	char m_cCONTC;
	char m_cSMART;
	char m_cPEFFT;
	char m_cDEFFT;
	char m_cTECPL;
	char m_cENRGY;
	float m_fOutputInterval;

private:


public:

	CCia3dOptionOutput()
	{
		m_cPRINT='n';
		m_cGDATA='n';
		m_cDISPL='n';
		m_cVELOC='n';
		m_cSTRES='n';
		m_cCONTC='n';
		m_cSMART='n';
		m_cPEFFT='n';
		m_cDEFFT='n';
		m_cTECPL='y';
		m_cENRGY='n';
		m_fOutputInterval = 1e-5f;
	}

	~CCia3dOptionOutput()
	{
		
	}

	bool loadFile(const char *fname);

	bool saveFile(const char *fname);

	bool loadXMLFile(const char *fname);

	void saveXMLFile(const char *fname);

	QDomDocument *createDomDocument(void);

	void convertFromDomDocument(QDomDocument *pdom);

};



extern void PrepareXmlSymbolTable(CCia3dOptionOutput &cio, CNameTableItem table[]);

#endif