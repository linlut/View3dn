//FILE: cia3dobj.cpp

#include "cia3dobj.h"
#include "tmeshobj.h"
#include "qmeshobj.h"
#include "pointobj.h"
#include "tetraobj.h"
#include "hexobj.h"
#include "lineobj.h"
#include "dynamic_array.h"
#include "volumemeshio.h"

using namespace std;


int LoadAndSetMultipleObjs(const char *fname, CCia3dObj * pCia3dObjs[], const int OBUFFLENGTH, const double factor)
{
	int rflag;
	const int BUFFLEN = 1024;
	CCia3dObj *pobjs[BUFFLEN], *p=NULL;
	for (int i=0; i<BUFFLEN; i++) pobjs[i]=NULL;

	//read the file
	try{
		p = new CCia3dObj;
		rflag = p->LoadFile(fname, factor);
		if (rflag==1){   
			if (p->ObjectCount()==1)
				pobjs[0] = p;
			else
				p->ObjectSplit(pobjs, OBUFFLENGTH);
		}
		else{ 
			delete p; p=NULL;
		}
	}
	catch(...){
		delete p;
		return false;
	}

	//test returning flag;
	if (rflag){
		for (int i=0; i<OBUFFLENGTH; i++){
			if (pCia3dObjs[i]) delete pCia3dObjs[i];  
			pCia3dObjs[i]=pobjs[i];
		}
	}
	return rflag;
}


int CCia3dObj::GetTotalTriangleCount(void)
{
	int i, ntri=0;
	int nobj = UnsplitObjectCount();
	for (i=0; i<nobj; i++){
		CObject3D * p = m_pObjList[i];
		const char *des = p->Description();
		if (strcmp(des, "tri") == 0){  //a triangle mesh;
			CTriangleObj *ptri = (CTriangleObj*)p;
			int nn = ptri->m_nPolygonCount;
			ntri+=nn;
		}
	}
	return ntri;
}


int CCia3dObj::_loadPltFile(const char *fileName)
{
	FILE* fp = fopen(fileName, _RB_);
	if (fp == NULL) return 0;

	AxisAlignedBox bbox, tbox;
	for (int nobj=0; nobj<100000; nobj++){
		float radius;
		int nvert, nface, nfattrib;
		char chunktype[2048]="\0";
		const int ATTRIBUTE_NUM = 100;
		char fattrib[ATTRIBUTE_NUM][9], ifattrib[ATTRIBUTE_NUM][9];
		int rflag = CPolyObj::ReadPltFileHeader(fp, chunktype, nvert, nface, radius, nfattrib, fattrib, ifattrib);
		if (!rflag) break;

		CPolyObj *ply=NULL;
		if (strstr(chunktype, "PARTICLE")||strstr(chunktype, "particle"))
			ply = new CPointObj;
		else if (strstr(chunktype, "TRIANGLE")||strstr(chunktype, "triangle"))
			ply =  new CTriangleObj;
		else if (strstr(chunktype, "QUADRILATERAL")||strstr(chunktype, "quadrilateral"))
			ply =  new CQuadObj;
		else if (strstr(chunktype, "LINE")||strstr(chunktype, "line"))
			ply =  new CLineObj;
		if (ply==NULL){
			fclose(fp);
			return 0;
		}

		//copy attrib strings and load mesh;
		if (nfattrib > CPolyObj::ATTRIBUTE_NUM)
			nfattrib = CPolyObj::ATTRIBUTE_NUM;
		for (int j=0; j<nfattrib; j++)
			strcpy(ply->m_strFVarNames[j], fattrib[j+3]);
		rflag = ply->LoadPltFileWithoutHeader(fp, nvert, nface, nfattrib);
		assert(rflag==1);

		//set bounding box
		ply->ComputeBoundingBox(tbox.minp, tbox.maxp);
		ply->SetBoundingBox(tbox);
		ply->m_fGivenRad = radius;

		//set name;
		ply->SetCustomizedObjectName(nobj);
		m_pObjList[nobj] = ply;
		bbox = Union(bbox, tbox);

		//set vertex ball radius;
		if (radius==0)
			ply->m_fGivenRad = ply->estimatedVertexRadius();
		else
			ply->m_fGivenRad = radius;
	}

	SetBoundingBox(bbox);
	//printf("Totally %d object(s) in the data file.\n", nobj);
	fclose(fp);
	return 1;
}

static CPolyObj * _loadNeutralMesh(const char *fname)
{
	CPolyObj *pobj=NULL;
	Vector3d* pVertex;
	int nv, nquad, nelm, meshtype, *pFace, *pCube;
	extern bool loadNeutralMeshFile(const char *fname, 
		Vector3d *& pVertex, int &nv, int *& pFace, int &ntri, int *& pTet, int &ntet, int &meshtype);

	if (!loadNeutralMeshFile(fname, pVertex, nv, pFace, nquad, pCube, nelm, meshtype))
		return NULL;
	assert(meshtype==2 || meshtype==1);
	if (meshtype==1){ //tet mesh
		CTetraObj *ptetobj  = new CTetraObj(pVertex, nv, (Vector4i*)pCube, nelm, (Vector3i*)pFace, nquad);
		assert(ptetobj!=NULL);
		pobj =ptetobj;
	}
	else if (meshtype==2){
		CHexObj *phexobj = new CHexObj(pVertex, nv, (Vector8i*)pCube, nelm, (Vector4i*)pFace, nquad);
		assert(phexobj!=NULL);
		pobj = phexobj;
	}
	return pobj;
}

static CPolyObj * _loadHMMesh(const char *fname)
{
	CPolyObj *pobj=NULL;
	Vector3d* pVertex;
	int nv, nquad, nelm, meshtype, *pFace, *pCube;

	if (!loadHMAsciiMeshFile(fname, pVertex, nv, pFace, nquad, pCube, nelm, meshtype))
		return NULL;
	assert(meshtype==2 || meshtype==1);
	if (meshtype==1){ //tet mesh
		CTetraObj *ptetobj  = new CTetraObj(pVertex, nv, (Vector4i*)pCube, nelm, (Vector3i*)pFace, nquad);
		assert(ptetobj!=NULL);
		pobj =ptetobj;
	}
	else if (meshtype==2){
		CHexObj *phexobj = new CHexObj(pVertex, nv, (Vector8i*)pCube, nelm, (Vector4i*)pFace, nquad);
		assert(phexobj!=NULL);
		pobj = phexobj;
	}
	return pobj;
}

static CPolyObj * _loadOffMesh(const char *fname)
{
	CPolyObj *pobj=NULL;
	Vector3d* pVertex;
	float *pIsoValue;
	int nv, nquad, nelm, meshtype, *pFace, *pCube;

	if (!loadOffMeshFile(fname, pVertex, nv, pIsoValue, pFace, nquad, pCube, nelm, meshtype))
		return NULL;
	assert(meshtype==1);  //tet mesh
	CTetraObj *ptetobj  = new CTetraObj(pVertex, nv, (Vector4i*)pCube, nelm, (Vector3i*)pFace, nquad);
	assert(ptetobj!=NULL);
	pobj =ptetobj;
	pobj->AddVertexAttributeFloat("IsoValue", pIsoValue);
	return pobj;
}


CPolyObj* loadNeutralMesh(const char *fname)
{
	return _loadNeutralMesh(fname);
}


CPolyObj* loadHMMesh(const char *fname)
{
	return _loadHMMesh(fname);
}


CPolyObj* loadOffMesh(const char *fname)
{
	return _loadOffMesh(fname);
}

static CPolyObj* loadPltFile(const char *fname)
{
	CCia3dObj obj;
	if (obj.LoadFile(fname, 1.0)){
		CPolyObj *p = dynamic_cast<CPolyObj*>(obj.m_pObjList[0]);
		if (p != NULL)
			obj.m_pObjList[0] = NULL;
		return p;
	}
	return NULL;
}

static bool _bool_loadPLTSurf = CPolyObjLoaderFactory::AddEntry(".plt", loadPltFile);
static bool _bool_loadNeutralMesh = CPolyObjLoaderFactory::AddEntry(".mesh", loadNeutralMesh);
static bool _bool_loadHMMesh = CPolyObjLoaderFactory::AddEntry(".hmascii", loadHMMesh);
static bool _bool_loadOffMesh = CPolyObjLoaderFactory::AddEntry(".offv", loadOffMesh);

int CCia3dObj::LoadFile(const char *fname, const double factor)
{
	const int strlength = strlen(fname);
	m_pObjList[0]=NULL;

	{//load plt file, where multiple objects can be loaded
		const int b1= strcmp(fname+(strlength-4), ".plt");
		const int b2= strcmp(fname+(strlength-4), ".PLT");
		if (b1==0 || b2==0){
			_loadPltFile(fname);
			goto EXITPOS;
		}
	}

	{//load plt file, where multiple objects can be loaded
		const int b1= strcmp(fname+(strlength-7), "/d3plot");
		const int b2= strcmp(fname+(strlength-7), "/D3PLOT");
		const int b3= strcmp(fname+(strlength-8), "/d3_plot");
		const int b4= strcmp(fname+(strlength-8), "/D3_PLOT");
		if (b1==0 || b2==0 || b3==0 || b4==0){
			_loadD3PlotFile(fname);
			goto EXITPOS;
		}
	}

	{//mesh reading successful?
	 //this factory method handles input with only one object input
		m_pObjList[0] = CPolyObjLoaderFactory::LoadPolygonMesh(fname);
	}

	if (m_pObjList[0]!=NULL){
		AxisAlignedBox box;
		m_pObjList[0]->GetBoundingBox(box);
		this->SetBoundingBox(box);	
	}

EXITPOS:
	//we may need to scale the object
	scaleObject(factor);
	return (m_pObjList[0]!=NULL);
}


int CCia3dObj::ObjectCount(int objidbuff[], const int bufflen)
{
	int i, j;
	CDynamicArray<int> A;

	for (i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
		CObject3D *p = m_pObjList[i];
		if (p==NULL) continue;
		int bsize= p->ObjectCount(objidbuff, bufflen);
		for (j=0; j<bsize; j++)
			A.AddWithNoDuplication(objidbuff[j]);
	}

	int asize = A.GetSize();
	assert(asize>0 && asize<bufflen);
	for (i=0; i<asize; i++)
		objidbuff[i] = A[i];
	return asize;
}


int CCia3dObj::ObjectSplit(CCia3dObj * buff[], const int bufflen)
{
	const int idsize = 2048;
	int objidbuff[idsize], objidbuff2[idsize];
	CObject3D * pobjs[idsize];
	
	const int n1 = ObjectCount(objidbuff, idsize);
	if (n1==1){
		printf("There is only one object, no need to split!\n");
		assert(0);
	}

	for (int i=0; i<n1; i++){
		buff[i] = new CCia3dObj;
		assert(buff[i]!=NULL);
	}

	for (int i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
		CObject3D *p = m_pObjList[i];
		if (p==NULL)
			break;
		const int n2 = p->ObjectCount(objidbuff2, idsize);
		if (n2==1){
			int key = objidbuff2[0];
			int index = SEARCH_KEY_POS(objidbuff, n1, key);
			buff[index]->addObject(p);
			p->SetCustomizedObjectName(index);
		}
		else{
			const int n3 = p->SplitIntoSubObj(objidbuff2, n2, pobjs);
			assert(n3==n2);
			for (int j=0; j<n2; j++){
				CObject3D * pobj = pobjs[j];
				int key = objidbuff2[j];
				int index = SEARCH_KEY_POS(objidbuff, n1, key);
				buff[index]->addObject(pobj);
				pobj->SetCustomizedObjectName(index);
			}
			delete p;
		}
		m_pObjList[i] = NULL;
	}

	return 1;
}


void CCia3dObj::scaleObject(const double factor)
{
	Vector3d lowleft, upright;
	CObject3D* p = NULL;

	for (int i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
		p = m_pObjList[i];
		if (p==NULL)  continue;
		CPolyObj *pobj = dynamic_cast<CPolyObj*>(p);
		if (pobj==NULL) continue;

		pobj->applyScaling(factor, factor, factor);
		pobj->ComputeBoundingBox(lowleft, upright);
		AxisAlignedBox box(lowleft, upright);
		pobj->SetBoundingBox(box);
	}
}


int CCia3dObj::_loadD3PlotFile(const char *fileName)
{
	assert(0);
	char chunktype[200], fattrib[35][9], ifattrib[35][9];
	int rflag, nvert, nface, nfattrib;
	float radius;
	//AxisAlignedBox bbox, tbox;

	//open the data file
	ifstream file(fileName, ios::in);
	if (!file.is_open()) return 0;
	if (file.bad() || file.fail()) return 0;

	std::string s;
	const int LINE_BUF_SIZE = 1024;
	char linebuff[LINE_BUF_SIZE];
	while (!file.eof()){
		file.getline(linebuff, LINE_BUF_SIZE-1);
		std::cout<< linebuff <<std::endl;
	}
	//SetBoundingBox(bbox);
	//std::cout << "Total " << i << " object(s) read in this data file.\n";

	//clean up the file operations
	file.close();

	return 0;
}

