/*------------------------------------------------------ 
//
//cia3dfile.cpp
//
//------------------------------------------------------*/

#include <memmgr.h>
#include "vertexinfo.h"
#include "cia3dfile.h"
#include "nametableitem.h"


static inline void SEEK_GROUPn(FILE *fp, const char *group1)
{
	char sbuffer[512];
	do{
		fgets(sbuffer, 512, fp);
		if (strstr(sbuffer, group1)!=NULL)
			break;
	}while(!feof(fp));
	assert(!feof(fp));
}


extern void __out_tecplot(
	long* ISTEP, double DTIME, double* DISPI,double* VELOI,double *STREP,
	double COORD[][4], long* MPOIN, long* NFACE, long* NSPON,
	long* ISURFV, long* NELEM, long* NOUTD,
	long* FIXED,long* VFIXD,long* IFPRE,long* EPSRA,double* DMONP,int* ID_OBJ,
	long* LD_SMART,long* LSETS,long* LNODT,long* LNODS);


void out_tecplot(
	long* ISTEP, double DTIME, double* DISPI,double* VELOI,double *STREP,
	double COORD[][4], long* MPOIN, long* NFACE, long* NSPON,
	long* ISURFV, long* NELEM, long* NOUTD,
	long* FIXED,long* VFIXD,long* IFPRE,long* EPSRA,double* DMONP,int* ID_OBJ,
	long* LD_SMART,long* LSETS,long* LNODT,long* LNODS
)
{
	double x, y, z, damage;
	char strbuff[256];
	char *group1="GROUP 1";
	static int count =0;
	int i, objid, nPoint, nElem, nMat, nStep, nAlgo, nMiter, nType;
	float fTol;
	FILE *fp;

/*
	printf("ISTEP = %d\n", *ISTEP);
	printf("MPOIN = %d\n", *MPOIN);
	printf("Nface = %d\n", *NFACE);
	printf("NSPON = %d\n", *NSPON);
	printf("ISURFV = %d\n", *ISURFV);
	printf("NELEM = %d\n", *NELEM);
	printf("NOUTD = %d\n", *NOUTD);
*/
	fp = fopen("./c3d1.in", "r");
	if (fp==NULL){
		printf("Cannot find CIA3D input file: c3d1.in\n");
		exit(1);
	}
	SEEK_GROUPn(fp, group1);
	fscanf(fp, "%d %d %d %d %d %d %lf %d\n", 
			    &nPoint, &nElem, &nMat, &nStep, &nAlgo, &nMiter, &fTol, &nType);
	fclose(fp);

	sprintf(strbuff, "./vtciaout%d.txt", count);
	fp = fopen(strbuff, "w");
	assert(fp!=NULL);
	fprintf(fp, "%d\n", nPoint);

	for (i=0; i<nPoint; i++){
		x = COORD[i][0];
		y = COORD[i][1];
		z = COORD[i][2];
		objid = ID_OBJ[i];
		damage = DMONP[i];
		fprintf(fp, "%d %lf %lf %lf %d %lf\n", i, x, y, z, objid, damage);
	}
	fclose(fp);
/*
	__out_tecplot(
		ISTEP, DTIME, DISPI,VELOI,STREP,
		COORD, MPOIN, NFACE, NSPON,
		ISURFV, NELEM, NOUTD,
		FIXED,VFIXD,IFPRE,EPSRA,DMONP,ID_OBJ,
		LD_SMART,LSETS,LNODT,LNODS);
*/
	count++;
}



//=======================Generate triangle boundary surface==============================
static CTriangleListItem* _searchTriangle(const int v0, const int v1, CTriangleListItem *head)
{
	CTriangleListItem *p=head;
	while (p!=NULL){
		if ((p->m_v0==v0 && p->m_v1==v1) || (p->m_v0==v1 && p->m_v1==v0))
			return p;
		p = p->m_pNext;
	}
	return NULL;
}

static void _insertTriangle(const Vector3i & tri, CTriangleListItem** plist, CMemoryMgr& mgr)
{
	int v0, v1, v2;
	v0 = _MIN3_(tri.x, tri.y, tri.z);
	if (v0==tri.x) v1=tri.y, v2=tri.z;
	else if (v0==tri.y) v1=tri.z, v2=tri.x;
	else v1=tri.x, v2=tri.y;

	CTriangleListItem* p = _searchTriangle(v1, v2, plist[v0]);
	if (p){
		p->m_v0=p->m_v1=-1;
		return;
	}

	CTriangleListItem * pListItem = (CTriangleListItem*)mgr.Malloc(sizeof(CTriangleListItem));
	pListItem->m_v0 = v1;
	pListItem->m_v1 = v2;
	pListItem->m_pNext = plist[v0];
	plist[v0] = pListItem;
}

static int _countValidTriangle(CTriangleListItem** plist, const int nv)
{
	int count = 0;
	for (int i=0; i<nv; i++){
		CTriangleListItem* p = plist[i];
		while (p!=NULL){
			if (p->m_v0!=-1)
				count++;
			p=p->m_pNext;
		}
	}
	return count;
}

static Vector3i* _collectBoundarySurface(CTriangleListItem** plist, const int nv, const int ntri)
{
	int c = 0;
	Vector3i *ptri = new Vector3i[ntri];
	if (ptri==NULL) return NULL;

	for (int i=0; i<nv; i++){
		CTriangleListItem *p = plist[i];
		while (p!=NULL){
			if (p->m_v0!=-1)
				ptri[c++]= Vector3i(i, p->m_v0, p->m_v1);
			p = p->m_pNext;
		}
	}
	assert(c==ntri);
	return ptri;
}

void computeBoundarySurface(const Vector4i *pTet, const int ntet, const int nvert, Vector3i *& pTri, int &nTri)
{
	int i;
	CMemoryMgr*  pm = new CMemoryMgr;

	//malloc the pointer buffer;
	CTriangleListItem** plist = new CTriangleListItem*[nvert];
	assert(plist!=NULL);
	for (i=0; i<nvert; i++) plist[i]=NULL;

	//insert triangles into the buffer;
	for (i=0; i<ntet; i++){
		Vector4i t = pTet[i];
		const int v0=t.x;
		const int v1=t.y;
		const int v2=t.z;
		const int v3=t.w;
		Vector3i tri0(v0, v2, v1);
		Vector3i tri1(v0, v1, v3);
		Vector3i tri2(v1, v2, v3);
		Vector3i tri3(v0, v3, v2);
		_insertTriangle(tri0, plist, *pm);
		_insertTriangle(tri1, plist, *pm);
		_insertTriangle(tri2, plist, *pm);
		_insertTriangle(tri3, plist, *pm);
	}
	
	nTri = _countValidTriangle(plist, nvert);
	pTri = _collectBoundarySurface(plist, nvert, nTri);
	delete pm;
	delete [] plist;
}


//=======================Generate quad boundary surface=========================

static CQuadListItem* _searchQuad(const int v0, const int v1, const int v2, CQuadListItem *head)
{
	CQuadListItem *p=head;
	Vector3i t0(v0,v1,v2);
	t0.bubbleSort();

	while (p!=NULL){
		Vector3i t1(p->m_v0, p->m_v1, p->m_v2);
		t1.bubbleSort();
		if (t0==t1)
			return p;
		p = p->m_pNext;
	}
	return NULL;
}

static void _insertQuad(const Vector4i & tri, CQuadListItem** plist, CMemoryMgr& mgr)
{
	int v0, v1, v2, v3;
	v0 = _MIN3_(tri.x, tri.y, tri.z);
	v0 = _MIN_(v0, tri.w);

	if (v0==tri.x) v1=tri.y, v2=tri.z, v3=tri.w;
	else if (v0==tri.y) v1=tri.z, v2=tri.w, v3=tri.x;
	else if (v0==tri.z) v1=tri.w, v2=tri.x, v3=tri.y;
	else v1=tri.x, v2=tri.y, v3=tri.z;

	CQuadListItem* p = _searchQuad(v1, v2, v3, plist[v0]);
	if (p){
		p->m_v0=-1;
		p->m_v1=-1;
		p->m_v2=-1;
		return;
	}
	CQuadListItem * pListItem = (CQuadListItem*)mgr.Malloc(sizeof(CQuadListItem));
	pListItem->m_v0 = v1;
	pListItem->m_v1 = v2;
	pListItem->m_v2 = v3;
	pListItem->m_pNext = plist[v0];
	plist[v0] = pListItem;
}

static int _countValidQuad(CQuadListItem** plist, const int nv)
{
	int count = 0;
	for (int i=0; i<nv; i++){
		CQuadListItem* p = plist[i];
		while (p!=NULL){
			if (p->m_v0!=-1) count++;
			p=p->m_pNext;
		}
	}
	return count;
}

static Vector4i* _collectBoundarySurface(CQuadListItem** plist, const int nv, const int ntri)
{
	int c = 0;
	Vector4i *ptri = new Vector4i[ntri];
	if (ptri==NULL) return NULL;

	for (int i=0; i<nv; i++){
		CQuadListItem *p = plist[i];
		while (p!=NULL){
			if (p->m_v0!=-1)
				ptri[c++]= Vector4i(i, p->m_v0, p->m_v1, p->m_v2);
			p = p->m_pNext;
		}
	}
	assert(c==ntri);
	return ptri;
}

void computeBoundarySurface(const Vector8i *pTet, const int ntet, const int nvert, Vector4i *& pTri, int &nTri)
{
	int i;
	CMemoryMgr*  pm = new CMemoryMgr;

	//malloc the pointer buffer;
	CQuadListItem** plist = new CQuadListItem*[nvert];
	assert(plist!=NULL);
	for (i=0; i<nvert; i++) plist[i]=NULL;

	//insert triangles into the buffer;
	for (i=0; i<ntet; i++){
		Vector8i t = pTet[i];
		const int v0=t.x;
		const int v1=t.y;
		const int v2=t.z;
		const int v3=t.w;
		const int v4=t.x1;
		const int v5=t.y1;
		const int v6=t.z1;
		const int v7=t.w1;
		Vector4i tri0(v0, v3, v2, v1);
		Vector4i tri1(v4, v5, v6, v7);
		Vector4i tri2(v0, v4, v7, v3);
		Vector4i tri3(v1, v2, v6, v5);
		Vector4i tri4(v0, v1, v5, v4);
		Vector4i tri5(v2, v3, v7, v6);
		_insertQuad(tri0, plist, *pm);
		_insertQuad(tri1, plist, *pm);
		_insertQuad(tri2, plist, *pm);
		_insertQuad(tri3, plist, *pm);
		_insertQuad(tri4, plist, *pm);
		_insertQuad(tri5, plist, *pm);
	}
	
	nTri = _countValidQuad(plist, nvert);
	pTri = _collectBoundarySurface(plist, nvert, nTri);
	delete pm;
	delete [] plist;
}



//=======================CC3d1inIOControl imp.==============================

void CC3d1inIOControl::_readConnectivity(FILE *fp)
{
	int i, id, matid, t0;
	int x0, y0, z0, w0, x1, y1, z1, w1;
	const char group3[]="GROUP 3";
	Vector4i *p4;
	Vector8i *p8;

	//malloc buffer for element id
	SafeDeleteArray(m_pElement);
	SafeDeleteArray(m_pElementObjectID);
	m_pElement = new Vector8i[m_nElement];
	assert(m_pElement!=NULL);
	m_pElementObjectID = new int [m_nElement];
	assert(m_pElementObjectID!=NULL);
	
	for (i=0; i<_CIA3D_MAX_OBJECT_COUNT; i++)
		m_nObjectElementCount[i] = 0;

	//read the elements;
	SEEK_GROUPn(fp, group3);
	for (i=0; i<m_nElement; i++){
		fscanf(fp, "%d %d %d", &id, &t0, &matid);

#ifdef _DEBUG
		if (id==2502){
			int asga=1;
		}
#endif

		m_pElementObjectID[i]= (--matid);	//set element matid
		m_nObjectElementCount[matid]++;		//count the element for this material

		const int nodecount = m_nObjectType[matid];
		switch(nodecount){
		case 4:
			fscanf(fp, "%d %d %d %d", &x0, &y0, &z0, &w0);
			x0--, y0--, z0--, w0--;
			p4 = (Vector4i*)(&m_pElement[i]);
			*p4 = Vector4i(x0, y0, z0, w0);
			break;
		case 8:
			fscanf(fp, "%d %d %d %d %d %d %d %d", &x0, &y0, &z0, &w0, &x1, &y1, &z1, &w1);
			x0--, y0--; z0--, w0--;
			x1--, y1--; z1--, w1--;	
			p8 = &m_pElement[i];
			*p8 = Vector8i(x0,y0,z0,w0,x1,y1,z1,w1);
			break;
		default:
			printf("Element type (%d nodes) is not supported!\n", nodecount);
			assert(0);
		}
	}
}


void CC3d1inIOControl::_readVertices(FILE *fp)
{
	m_pVertex = new Vector3d[m_nVertex];
	assert(m_pVertex!=NULL);

	//read all the vertices;
	for (int i=0; i<m_nVertex; i++){
		int id;
		double thickness;
		Vector3d *p = &m_pVertex[i];
		fscanf(fp, "%d %lf %lf %lf %lf", &id, &p->x, &p->y, &p->z, &thickness);
	}
}


bool CC3d1inIOControl::loadC3dinFile(const char *fname)
{
	int i, tmp1, tmp2;
	const char group1[]="GROUP 1";
	char sbuffer[516];
	FILE *fp = fopen(fname, _RA_);
	if (fp==NULL) return false;

	//read the type of problem to solve
	fscanf(fp, "%d", &m_nIStat);
	assert(m_nIStat>=0 && m_nIStat<10);

	//read the first group, skip comments
	SEEK_GROUPn(fp, group1);
	fscanf(fp, "%d %d %d %d %d %d %lf %d\n", 
			    &m_nVertex, &m_nElement, &m_nMaterial, &m_nStep, 
				&m_nAlgo, &m_nMiter, &m_fTol, &m_nType);
	assert(m_nType==1);			//Only support 1 type at this time;

	fgets(sbuffer, 512, fp);	//skip parameters of outputs

	//read parameters for boundary ,initial and formulation options
	fscanf(fp, "%d %d %d", &m_nVfix, &m_nPrev, &m_nLaps);

	//read node count for each element type
	fscanf(fp, "%d %d", &m_nAlgType, &tmp2);		//read mat set,only 1 set, then tmp2=total mat;
	assert(m_nAlgType==1 || m_nAlgType==4 ||m_nAlgType==5);
	assert(tmp2==m_nMaterial);
	fscanf(fp, "%d", &tmp1);
	assert(tmp1==m_nAlgType);

	for (i=0; i<m_nMaterial; i++){
		int tmp1, tmp2, tmp3, tmp4, tmp5, nNode;
		fscanf(fp, "%d %d %d %d %d &d", &tmp1, &tmp2, &nNode, &tmp3, &tmp4, &tmp5);
		tmp4--;
		m_nObjectType[tmp4] = nNode; 
	}

	//read mesh connectivity and vertices;
	_readConnectivity(fp);
	_readVertices(fp);

	printf("File <%s> reading successful.\n", fname);
	printf("\tVertex number: %d, Element number: %d\n", m_nVertex, m_nElement);
	return true;
}


int CC3d1inIOControl::_getVertexCount(const int matid)
{
	if (m_pElement==NULL || m_pElementObjectID==NULL) return 0;
	int count=0, i;
	const int elmtype = m_nObjectType[matid];
	int* const v = new int [m_nVertex];
	assert(v!=NULL);
	for (i=0; i<m_nVertex; i++) v[i]=-1;

	//test the data
	const Vector4i* p4=NULL;
	const Vector8i* p8=NULL;
	for (i=0; i<m_nElement; i++){
		if (m_pElementObjectID[i]==matid){
			switch(elmtype){
			case 4:
				p4 = (Vector4i*)&m_pElement[i];
				v[p4->x]=matid;v[p4->y]=matid;
				v[p4->z]=matid;v[p4->w]=matid;
				break;
			case 8: 
				p8 = (Vector8i*)&m_pElement[i];
				v[p8->x]=matid;v[p8->y]=matid;
				v[p8->z]=matid;v[p8->w]=matid;
				v[p8->x1]=matid;v[p8->y1]=matid;
				v[p8->z1]=matid;v[p8->w1]=matid;
				break;
			default:
				assert(0);
			}
		}
	}

	for (i=0; i<m_nVertex; i++)
		if (v[i]==matid) count++;
	delete [] v;
	return count;
}


void CC3d1inIOControl::checkVertexWithNoMatID(void)
{
	if (m_pElement==NULL || m_pElementObjectID==NULL) return;
	int count=0, i;
	int* const v = new int [m_nVertex];
	assert(v!=NULL);
	for (i=0; i<m_nVertex; i++) v[i]=-1;

	//test the data
	const Vector4i* p4=NULL;
	const Vector8i* p8=NULL;
	for (i=0; i<m_nElement; i++){
		const int matid = m_pElementObjectID[i];
		const int elmtype = m_nObjectType[matid];
		switch(elmtype){
			case 4:
				p4 = (Vector4i*)&m_pElement[i];
				v[p4->x]=matid;v[p4->y]=matid;
				v[p4->z]=matid;v[p4->w]=matid;
				break;
			case 8: 
				p8 = (Vector8i*)&m_pElement[i];
				v[p8->x]=matid;v[p8->y]=matid;
				v[p8->z]=matid;v[p8->w]=matid;
				v[p8->x1]=matid;v[p8->y1]=matid;
				v[p8->z1]=matid;v[p8->w1]=matid;
				break;
			default:
				assert(0);
		}
	}

	for (i=0; i<m_nVertex; i++)
		if (v[i]==-1) count++;
	delete [] v;
	printf("This data has %d vertices without material.\n", count);
}



void CC3d1inIOControl::getSingleObject(const int objid, Vector3d *& pVertex, int &nVertex, void* &pElement, int &nElement)
{
	int i, j, c;
	Vector4i *pe4=NULL;
	Vector8i *pe8=NULL;

	//Make sure the object is a tet mesh;
	assert(objid>=0 && objid<m_nMaterial);
	const int elmtype = m_nObjectType[objid];
	assert(elmtype==4 || elmtype==8);

	const int ne = _getElementCount(objid);
	const int nv = _getVertexCount(objid);
	pVertex = new Vector3d[nv];
	if (elmtype==4){
		pe4 = new Vector4i[ne];
		assert(pVertex!=NULL && pe4!=NULL);
	}
	else{
		pe8 = new Vector8i[ne];
		assert(pVertex!=NULL && pe8!=NULL);
	}

	int *pindex = new int [m_nVertex];
	assert(pindex!=NULL);
	for (i=0; i<m_nVertex; i++) pindex[i]=-1;

	//copy the elements;
	for (i=j=c=0; i<m_nElement; i++){
		if (m_pElementObjectID[i]==objid){
			if (elmtype==4){
				Vector4i *p4 = (Vector4i*) &m_pElement[i];
				const int v0 = p4->x;
				if (pindex[v0]==-1) pindex[v0]=c++;
				const int v1 = p4->y;
				if (pindex[v1]==-1) pindex[v1]=c++;
				const int v2 = p4->z;
				if (pindex[v2]==-1) pindex[v2]=c++;
				const int v3 = p4->w;
				if (pindex[v3]==-1) pindex[v3]=c++;
				pe4[j++] = Vector4i(pindex[v0], pindex[v1], pindex[v2], pindex[v3]);
			}
			else{
				Vector8i *p8 = (Vector8i*) &m_pElement[i];
				const int v0 = p8->x;
				if (pindex[v0]==-1) pindex[v0]=c++;
				const int v1 = p8->y;
				if (pindex[v1]==-1) pindex[v1]=c++;
				const int v2 = p8->z;
				if (pindex[v2]==-1) pindex[v2]=c++;
				const int v3 = p8->w;
				if (pindex[v3]==-1) pindex[v3]=c++;
				const int v4 = p8->x1;
				if (pindex[v4]==-1) pindex[v4]=c++;
				const int v5 = p8->y1;
				if (pindex[v5]==-1) pindex[v5]=c++;
				const int v6 = p8->z1;
				if (pindex[v6]==-1) pindex[v6]=c++;
				const int v7 = p8->w1;
				if (pindex[v7]==-1) pindex[v7]=c++;
				pe8[j++] = Vector8i(pindex[v0], pindex[v1], pindex[v2], pindex[v3], 
					                pindex[v4], pindex[v5], pindex[v6], pindex[v7]);
			}
		}
	}
	ASSERT0(c==nv && j==ne);	 //varify the alg.

	//copy the vertices;
	for (i=0; i<m_nVertex; i++){
		const int pos = pindex[i];
		if (pos!=-1)
			pVertex[pos] = m_pVertex[i];
	}

	nVertex = nv, nElement = ne;
	if (elmtype==4) pElement = pe4;
	else pElement=pe8;
	delete [] pindex;
}



bool CC3d1inIOControl::loadXMLFile(const char *fname)
{
	return true;
}


void CC3d1inIOControl::saveXMLFile(const char *fname)
{
	return;
}



//===================================================================

QDomDocument *CC3d1inIOControl::createDomDocument(void)
{
	int i, slen = 0;
	char *sbuffer = new char [3048]; sbuffer[0]=0;
	const char *group1 = "ALGORITHM     ";
	const char *group2 = "TOLERANCE     ";
	char *name, *type, *val, *option, *comm;

	QDomDocument * pdom = new QDomDocument();
	assert(pdom!=NULL);
	CNameTableItem table[32];
	PrepareXmlSymbolTable(*this, table);

	//write header
	sprintf(&sbuffer[slen], "%s", "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"); slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<body>\n");slen=strlen(sbuffer);
	sprintf(&sbuffer[slen], "<xmldlg version=\"1.0\">\n");slen=strlen(sbuffer);

	//write group 1, 2
	beginXMLCLASS(sbuffer, slen, group1);
	for (i=0; i<2; i++){
		CNameTableItem* p = &table[i];
		if (p->isNull()) break;
		p->toString(name, type, val, option, comm);
		writeXMLItem(sbuffer, slen, name, type, val, option, comm);
	}
	endXMLCLASS(sbuffer, slen, group1);

	beginXMLCLASS(sbuffer, slen, group2);
	for (i=2; i<30; i++){
		CNameTableItem* p = &table[i];
		if (p->isNull()) break;
		p->toString(name, type, val, option, comm);
		writeXMLItem(sbuffer, slen, name, type, val, comm);
	}
	endXMLCLASS(sbuffer, slen, group2);

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


void CC3d1inIOControl::convertFromDomDocument(QDomDocument *pdom)
{
	int i;
	if (pdom==NULL) return;

	QDomElement root = pdom->documentElement();
	CNameTableItem table[30];
	PrepareXmlSymbolTable(*this, table);

	for (i=0;i<30;i++){
		CNameTableItem *p = &table[i];
		if (p->isNull()) break;
		QDomNode * pnode = _searchItemNodeByName(root, p->m_sName);
		if (pnode){
			QString text = pnode->toElement().attribute("value");
			//puts(text);
			p->setValue(text);
		}
	}
}



void PrepareXmlSymbolTable(CC3d1inIOControl &cio, CNameTableItem table[])
{
	int i = 0;
	static char *m_nIStat_option = 
								 "1: Static Problem,"
								 "2: Dynamic Problem,"
								 "3: Static Contact Problem,"
								 "4: Dynamic Contact/Impact Problem,"
								 "5: Dynamic Penetration Problem";
	static char *m_nIStat_comm = "See the popup menu for choices";
	static char *m_nAlgType_option = 
								 "1: Finite Element Method,"
								 "2: Don't Use,"
								 "3: Don't Use,"
								 "4: Meshfree Method,"
								 "5: Finite Mass Method";
	static char *m_nAlgType_comm = "See the popup menu for choices";

	//Option for analysis package
	table[i++]=CNameTableItem("Problem Type", 'm', &cio.m_nIStat, m_nIStat_option, m_nIStat_comm);
	table[i++]=CNameTableItem("Element Type", 'm', &cio.m_nAlgType, m_nAlgType_option, m_nAlgType_comm);

	table[i++]=CNameTableItem("Time Steps", 'i', &cio.m_nStep);
	table[i++]=CNameTableItem("NALGO", 'i', &cio.m_nAlgo);
	table[i++]=CNameTableItem("Max # of Iterations ", 'i', &cio.m_nMiter);
	table[i++]=CNameTableItem("Zero Tolerence", 'd', &cio.m_fTol);
	table[i++]=CNameTableItem("NTYPE", 'i', &cio.m_nType);

//	table[i++]=CNameTableItem("NVFIX", 'i', &cio.m_nVfix);
	table[i++]=CNameTableItem("NPREV", 'i', &cio.m_nPrev);
	table[i++]=CNameTableItem("NLAPS", 'i', &cio.m_nLaps);

	table[i++]=CNameTableItem("Friction Ratio", 'd', &cio.m_fFrict);
	table[i++]=CNameTableItem("Friction Contact Method ", 'i', &cio.m_nMethod);
	table[i++]=CNameTableItem("ISEAH", 'i', &cio.m_nIseah);
	table[i++]=CNameTableItem("REQC", 'i', &cio.m_nReqc);
	table[i++]=CNameTableItem("Penalty Normal", 'd', &cio.m_fPenan);
	table[i++]=CNameTableItem("Penalty Tengential", 'd', &cio.m_fPenat);

	table[i++]=CNameTableItem("", 'i', NULL);

	/*
		int m_nIStat;		//Option for analysis package
		m_nStep=200000;	//load step or time step
		m_nAlgo=1;		//indicator of Newton Raphson Iteration
		m_nMiter=30;	//max number of iteration
		m_fTol=1e-9;
		m_nType = 1;	//one element type

		m_nVfix=0;		//number of fixed vertices;
		m_nPrev=0;		//indicator of initial stree
		m_nLaps=3;		//indicator for finite formulation, see cia3d manual
						//0: linear problem
						//1: elasto-platic large deformation
						//2: elastic large deformation by T.L.F.
						//3: elasto-plastic large deformation by T.L.R.
						//4, 5, .....

		m_fFrict=0;		//coefficient of Coulmb friction
		m_nMethod=0;	//indicator of frictional contact method
		m_nIseah=0;		//indicator of contact surface reach	
		m_nReqc=0;		//gloabl point to output
		m_fPenan=0;		//penalty for normal direction
		m_fPenat=0;		//penalty for tengential direction
		*/
}
