/*------------------------------------------------------ 
//
//op_output.cpp
//
//------------------------------------------------------*/

#include "op_output.h"


bool CCia3dOptionOutput::loadFile(const char *fname)
{
	FILE *fp = fopen(fname, _RA_);
	if (fp==NULL) return false;
	
	fscanf(fp, "PRINT.OUT: %c\n", &m_cPRINT);
	fscanf(fp, "GDATA.OUT: %c\n", &m_cGDATA);
	fscanf(fp, "DISPL.OUT: %c\n", &m_cDISPL);
	fscanf(fp, "VELOC.OUT: %c\n", &m_cVELOC);

	fscanf(fp, "STRES.OUT: %c\n", &m_cSTRES);
	fscanf(fp, "CONTC.OUT: %c\n", &m_cCONTC);
	fscanf(fp, "SMART.OUT: %c\n", &m_cSMART);
	fscanf(fp, "PEFFT.OUT: %c\n", &m_cPEFFT);

	fscanf(fp, "DEFFT.OUT: %c\n", &m_cDEFFT);
	fscanf(fp, "TECPL.OUT: %c\n", &m_cTECPL);
	fscanf(fp, "ENRGY.OUT: %c\n", &m_cENRGY);
	fscanf(fp, "%f\n", &m_fOutputInterval);

	fclose(fp);
	return true;
}

bool CCia3dOptionOutput::saveFile(const char *fname)
{
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return false;
	
	fprintf(fp, "PRINT.OUT: %c\n", m_cPRINT);
	fprintf(fp, "GDATA.OUT: %c\n", m_cGDATA);
	fprintf(fp, "DISPL.OUT: %c\n", m_cDISPL);
	fprintf(fp, "VELOC.OUT: %c\n", m_cVELOC);

	fprintf(fp, "STRES.OUT: %c\n", m_cSTRES);
	fprintf(fp, "CONTC.OUT: %c\n", m_cCONTC);
	fprintf(fp, "SMART.OUT: %c\n", m_cSMART);
	fprintf(fp, "PEFFT.OUT: %c\n", m_cPEFFT);

	fprintf(fp, "DEFFT.OUT: %c\n", m_cDEFFT);
	fprintf(fp, "TECPL.OUT: %c\n", m_cTECPL);
	fprintf(fp, "ENRGY.OUT: %c\n", m_cENRGY);
	fprintf(fp, "%g\n", m_fOutputInterval);

	fclose(fp);
	return true;
}


bool CCia3dOptionOutput::loadXMLFile(const char *fname)
{
	return true;
}

void CCia3dOptionOutput::saveXMLFile(const char *fname)
{
	return ;
}

static const int TABBUFFLEN=12;
QDomDocument* CCia3dOptionOutput::createDomDocument(void)
{
	int i, slen = 0;
	char *sbuffer = new char [3048]; sbuffer[0]=0;
	char *name, *type, *val, *option, *comm;

	QDomDocument * pdom = new QDomDocument();
	assert(pdom!=NULL);
	CNameTableItem table[TABBUFFLEN+2];
	PrepareXmlSymbolTable(*this, table);

	//write header
	sprintf(&sbuffer[slen], "%s", "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"); slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<body>\n");slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<xmldlg version=\"1.0\">\n");slen=strlen(sbuffer);

	//write group 1, 2
	for (i=0; i<TABBUFFLEN; i++){
		CNameTableItem* p = &table[i];
		if (p->isNull()) break;
		p->toString(name, type, val, option, comm);
		writeXMLItem(sbuffer, slen, name, type, val, option, comm);
	}

	//write tail
	sprintf(&sbuffer[slen], "</xmldlg>\n");slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "</body>\n");slen=strlen(sbuffer);

	//set content for the DomDocument
	//puts(sbuffer);
	QString qstr(sbuffer), qerrmsg;
	int errorLine=0, errorColumn=0; 
	pdom->setContent(qstr, &qerrmsg, &errorLine, &errorColumn);
	//printf("Error msg is %s (line: %d, col: %d)\n", qerrmsg.ascii(), errorLine, errorColumn);
	delete sbuffer;
	return pdom;
}

void CCia3dOptionOutput::convertFromDomDocument(QDomDocument *pdom)
{
	int i;
	if (pdom==NULL) return;

	QDomElement root = pdom->documentElement();
	CNameTableItem table[TABBUFFLEN+2];
	PrepareXmlSymbolTable(*this, table);

	for (i=0;i<TABBUFFLEN;i++){
		CNameTableItem *p = &table[i];
		if (p->isNull()) break;
		QDomNode * pnode = _searchItemNodeByName(root, p->m_sName);
		if (pnode){
			QString text = pnode->toElement().attribute("value");
			p->setValue(text);
		}
	}
}

void PrepareXmlSymbolTable(CCia3dOptionOutput &cio, CNameTableItem table[])
{
	int i = 0;
	static char* m_cPRINT="Print output";				//0
	static char* m_cGDATA="Output grid data";			//1
	static char* m_cDISPL="Output displacement";		//2
	static char* m_cVELOC="Output velocity";			//3

	static char* m_cSTRES="Output stress for each node";//4
	static char* m_cCONTC="Output contact info.";		//5
	static char* m_cSMART="Smart switch";				//6
	static char* m_cPEFFT="not clear";					//7

	static char* m_cDEFFT="not clear";					//8
	static char* m_cTECPL="Output result in TecPlot";	//9
	static char* m_cENRGY="Output nodal energy info.";	//10
	static char* m_fOutputInterval="Output interval";	//11

	table[i++]=CNameTableItem("PRINT", 'b', (int*)(&cio.m_cPRINT), NULL, m_cPRINT);
	table[i++]=CNameTableItem("GDATA", 'b', (int*)(&cio.m_cGDATA), NULL, m_cGDATA);
	table[i++]=CNameTableItem("DISPL", 'b', (int*)(&cio.m_cDISPL), NULL, m_cDISPL);
	table[i++]=CNameTableItem("VELOC", 'b', (int*)(&cio.m_cVELOC), NULL, m_cVELOC);

	table[i++]=CNameTableItem("STRESS", 'b', (int*)(&cio.m_cSTRES), NULL, m_cSTRES);
	table[i++]=CNameTableItem("CONTACT", 'b', (int*)(&cio.m_cCONTC), NULL, m_cCONTC);
	table[i++]=CNameTableItem("SMART SWITCH", 'b', (int*)(&cio.m_cSMART), NULL, m_cSMART);
	table[i++]=CNameTableItem("PEFFT", 'b', (int*)(&cio.m_cPEFFT), NULL, m_cPEFFT);

	table[i++]=CNameTableItem("DEFFT", 'b', (int*)(&cio.m_cDEFFT), NULL, m_cDEFFT);
	table[i++]=CNameTableItem("TECPLOT", 'b', (int*)(&cio.m_cTECPL), NULL, m_cTECPL);
	table[i++]=CNameTableItem("ENERRGY", 'b', (int*)(&cio.m_cENRGY), NULL, m_cENRGY);
	table[i++]=CNameTableItem("OUTPUT INTERVAL(sec.)    ", 'f', (int*)(&cio.m_fOutputInterval), NULL, m_fOutputInterval);
}
