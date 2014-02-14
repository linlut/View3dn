/*------------------------------------------------------ 
//
//op_imp.cpp
//
//------------------------------------------------------*/

#include "op_imp.h"


bool CCia3dOptionImp::loadFile(const char *fname)
{
	return true;
}

bool CCia3dOptionImp::saveFile(const char *fname)
{
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return false;
	fprintf(fp, "%d\n", m_nIDFormulation);
	fprintf(fp, "%d\n", m_nTimeIntegrationAlg);

	fprintf(fp, "%g %g\n", m_fG_MAX, m_fV_DOT_N_MIN);
	fprintf(fp, "%d\n", m_nContactSearch);
	fprintf(fp, "%d\n",	m_nLSWITCH);
	fprintf(fp, "%d\n", m_nLSOLV);
	fprintf(fp, "%c\n", m_cAdaptive);
	fprintf(fp, "%g\n", m_fFactor);
	fprintf(fp, "%g\n", m_fGAP_SW);

	fclose(fp);
	return true;
}


bool CCia3dOptionImp::loadXMLFile(const char *fname)
{
	return true;
}

void CCia3dOptionImp::saveXMLFile(const char *fname)
{
	return ;
}

static	const int BUFFLENGTH=10;
QDomDocument* CCia3dOptionImp::createDomDocument(void)
{
	int i, slen = 0;
	char *sbuffer = new char [3048]; sbuffer[0]=0;
	char *name, *type, *val, *option, *comm;

	QDomDocument * pdom = new QDomDocument();
	assert(pdom!=NULL);
	CNameTableItem table[BUFFLENGTH+2];
	PrepareXmlSymbolTable(*this, table);

	//write header
	sprintf(&sbuffer[slen], "%s", "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"); slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<body>\n");slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<xmldlg version=\"1.0\">\n");slen=strlen(sbuffer);

	//write group 1, 2
	//const char *group1 = "Dynamics Options";
	//beginXMLCLASS(sbuffer, slen, group1);
	for (i=0; i<BUFFLENGTH; i++){
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

void CCia3dOptionImp::convertFromDomDocument(QDomDocument *pdom)
{
	int i;
	if (pdom==NULL) return;

	QDomElement root = pdom->documentElement();
	CNameTableItem table[BUFFLENGTH+2];
	PrepareXmlSymbolTable(*this, table);

	for (i=0;i<BUFFLENGTH;i++){
		CNameTableItem *p = &table[i];
		if (p->isNull()) break;
		QDomNode * pnode = _searchItemNodeByName(root, p->m_sName);
		if (pnode){
			QString text = pnode->toElement().attribute("value");
			p->setValue(text);
		}
	}
}

void PrepareXmlSymbolTable(CCia3dOptionImp &cio, CNameTableItem table[])
{
	int i = 0;
	static char *strIDFormulation = 
		"1: Total Lagrangian formulation (TLF),"
		"2: Arbitrary reference configuration Lagrangian formulation (ARCLF) and pure explicit FDCD";
	static char *strIDFormulation_comm = "See the popup menu for choices";
	static char *strTimeIntegrationAlg = 
		"1: DO NOT USE. Newmark average acceleration method with full Newton-Raphson iteration,"
		"2: DO NOT USE. Newmark average acceleration method with modified Newton-Raphson iteration,"
		"3: DO NOT USE. Forward incremental displacement central difference (FIDCD method),"
		"4: DO NOT USE. (OLD EL method),"
		"5: Forward displacement second-order accurate nonlinearly explicit Lstable (FDEL) method, and pure explicit FDCD";
	static char *strTimeIntegrationAlg_comm = "See the popup menu for choices";

	static char *strG_MAX="Auto search radius in [0, +INF]";
	static char *strV_DOT_N_MIN="DotProd between object initial velocity and master segment normal direction, in [-1.0, 1.0]";

	static char *strContactSearch=
		"1: only search contact between bodies,"
		"2: include self-contact search";

	static char *strLSWITCH=
		"1: no switch,"
		"2: stupid switch: given time step auto select FDCD or FDEL,"
		"3: contact switch: auto select FDCD with critical time step for contact and select FDEL with time step FACTOR*tcritical for dynamic response,"
		"4: element based smart switch: in element based formulation auto select FDCD with critical time step tcritical and FDEL with time step," 
		"5: smart switch: auto select FDCD with critical time step tcritical and FDEL";

	static char *strLSOLV="valid only when m_nTimeIntegrationAlg is 5";
	//LSOLV2 {1, 2, 3}
	//1 FDEL family: conditional stable FDCD
	//2 FDEL family: weak stable
	//3 FDEL family: strong stable

	static char* strAdaptive= "Adaptive search (valid only when m_nTimeIntegrationAlg is 5)";
	//y using adaptive time steping, automatic calculate critical time step
	//n using the fixed time step provided in c3d1.in
	
	static char* strFactor="Factor of time step, requires m_nTimeIntegrationAlg=5 AND m_cAdaptive=y";
	//FACTOR in [0, +INF]
	//4.0 the factor of time step for EL in the unit of CD critical time step

	static char* strGAP_SW="Gap"; //IF m_nLSWITCH=3
	//GAP SW2 [0,+INF]
	//0.0001 when minimum gap distant greater than GAP SW, use FDEL, otherwise use FDCD

	//Option for analysis package
	table[i++]=CNameTableItem("IDFormulation", 'm', &cio.m_nIDFormulation, strIDFormulation, strIDFormulation_comm);
	table[i++]=CNameTableItem("TimeIntegration Alg.", 'm', &cio.m_nTimeIntegrationAlg, strTimeIntegrationAlg, strTimeIntegrationAlg_comm);
	table[i++]=CNameTableItem("AutoSearch Radius", 'f', (int*)(&cio.m_fG_MAX), strG_MAX);
	table[i++]=CNameTableItem("DotProd", 'f', (int*)(&cio.m_fV_DOT_N_MIN), strV_DOT_N_MIN);
	table[i++]=CNameTableItem("Contact Search Method   ", 'm', &cio.m_nContactSearch, strContactSearch);
	table[i++]=CNameTableItem("Family Switch", 'm', &cio.m_nLSWITCH, strLSWITCH);

	table[i++]=CNameTableItem("FDEL Time Int.", 'm', &cio.m_nLSOLV, strLSOLV);
	table[i++]=CNameTableItem("Adaptive Comp.", 'b', (int*)(&cio.m_cAdaptive), strAdaptive);

	table[i++]=CNameTableItem("Factor of Time Step", 'f', (int*)(&cio.m_fFactor), strFactor);
	table[i++]=CNameTableItem("Gap", 'f', (int*)(&cio.m_fGAP_SW), strGAP_SW);
}
