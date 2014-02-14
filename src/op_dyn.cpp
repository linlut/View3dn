/*------------------------------------------------------ 
//
//op_dyn.cpp
//
//------------------------------------------------------*/

#include "op_dyn.h"


bool CCia3dOptionDyn::loadFile(const char *fname)
{
	return true;
}

bool CCia3dOptionDyn::saveFile(const char *fname)
{
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return false;
	fprintf(fp, "%d\n", m_nIDFormulation);
	fprintf(fp, "%d\n", m_nTimeIntegrationAlg);
	fclose(fp);
	return true;
}


bool CCia3dOptionDyn::loadXMLFile(const char *fname)
{
	return true;
}

void CCia3dOptionDyn::saveXMLFile(const char *fname)
{
	return ;
}

QDomDocument* CCia3dOptionDyn::createDomDocument(void)
{
	int i, slen = 0;
	char *sbuffer = new char [3048]; sbuffer[0]=0;
	char *name, *type, *val, *option, *comm;

	QDomDocument * pdom = new QDomDocument();
	assert(pdom!=NULL);
	CNameTableItem table[10];
	PrepareXmlSymbolTable(*this, table);

	//write header
	sprintf(&sbuffer[slen], "%s", "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"); slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<body>\n");slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<xmldlg version=\"1.0\">\n");slen=strlen(sbuffer);

	//write group 1, 2
	//const char *group1 = "Dynamics Options";
	//beginXMLCLASS(sbuffer, slen, group1);
	for (i=0; i<2; i++){
		CNameTableItem* p = &table[i];
		if (p->isNull()) break;
		p->toString(name, type, val, option, comm);
		writeXMLItem(sbuffer, slen, name, type, val, option, comm);
	}
	//endXMLCLASS(sbuffer, slen, group1);

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

void CCia3dOptionDyn::convertFromDomDocument(QDomDocument *pdom)
{
	int i;
	if (pdom==NULL) return;

	QDomElement root = pdom->documentElement();
	CNameTableItem table[10];
	PrepareXmlSymbolTable(*this, table);

	for (i=0;i<10;i++){
		CNameTableItem *p = &table[i];
		if (p->isNull()) break;
		QDomNode * pnode = _searchItemNodeByName(root, p->m_sName);
		if (pnode){
			QString text = pnode->toElement().attribute("value");
			p->setValue(text);
		}
	}
}

void PrepareXmlSymbolTable(CCia3dOptionDyn &cio, CNameTableItem table[])
{
	int i = 0;
	static char *strIDFormulation = 
		"1: Total Lagrangian formulation (TLF),"
		"2: Arbitrary reference configuration Lagrangian formulation (ARCLF) and pure explicit FDCD";
	static char *strIDFormulation_comm = "See the popup menu for choices";
	static char *strTimeIntegrationAlg = 
		"1: Newmark average acceleration method with full Newton-Raphson iteration,"
		"2: Newmark average acceleration method with modified Newton-Raphson iteration,"
		"3: Forward incremental displacement central difference (FIDCD method,"
		"4: DO NOT USE (OLD EL method),"
		"5: Forward displacement second-order accurate nonlinearly explicit Lstable (FDEL) method";
	static char *strTimeIntegrationAlg_comm = "See the popup menu for choices";

	//Option for analysis package
	table[i++]=CNameTableItem("IDFormulation", 'm', &cio.m_nIDFormulation, strIDFormulation, strIDFormulation_comm);
	table[i++]=CNameTableItem("TimeIntegrationAlg    ", 'm', &cio.m_nTimeIntegrationAlg, strTimeIntegrationAlg, strTimeIntegrationAlg_comm);

}
