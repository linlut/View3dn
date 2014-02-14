//FILE: meshspheredialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"
#include "meshtetradialog.h"



//=======================================================================
static void 
sphereSubzionToHexObj(MV3& vertex, const int nx, CHexObj &obj)
{ 
	int _tmp, ny, nz, c;
	vertex.getMatrixDimension(_tmp, ny, nz);
	assert(_tmp>=nx);
	const int nsize = nx*ny*nz;
	Vector3d *meshvert = new Vector3d[nsize];
	assert(meshvert!=NULL);

	//copy the first layer vertices
	c = 0;
	for (int k=1; k<=nz; k++){
		for (int j=1; j<=ny; j++){
			for (int i=1; i<=nx; i++, c++)
				meshvert[c] = vertex(i, j, k);
		}
	}
	obj.m_nVertexCount = nsize;
	obj.m_pVertex = meshvert;
	Vector8i *&objm_pHex = (Vector8i *&)obj.m_pElement;
	getBrickElements(nx-1, ny-1, nz-1, objm_pHex, obj.m_nElementCount);
	Vector4i * &m_pQuad = (Vector4i * &)obj.m_pPolygon;
	getBrickBounday(nx-1, ny-1, nz-1, m_pQuad, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
}


static CHexObj* 
meshTetrahedron(const int divx, const int divy, const int divz, const Vector3d v[4])
{
	//create the sphere elements
	MV3 vertex;
	tetrahedronMesher(divx, divy, divz, v, vertex);

	//separate the 3 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx4 = nx/4;
	const int nsize = nx4*ny*nz;
	assert(nx%4==0);
	MV3 obj1=vertex(1);
	MV3 obj2=vertex(1+nx4);
	MV3 obj3=vertex(1+nx4*2);
	MV3 obj4=vertex(1+nx4*3);

	CHexObj tmpObj1, tmpObj2;
	{
		CHexObj *pobj1 = new CHexObj;
		CHexObj *pobj2 = new CHexObj;
		assert(pobj1!=NULL && pobj2!=NULL);
		sphereSubzionToHexObj(obj1, nx4, *pobj1);
		sphereSubzionToHexObj(obj2, nx4, *pobj2);
		MergeHexObj(*pobj1, *pobj2, tmpObj1);
		delete pobj1;
		delete pobj2;
	}
	{
		CHexObj *pobj1 = new CHexObj;
		CHexObj *pobj2 = new CHexObj;
		assert(pobj1!=NULL && pobj2!=NULL);
		sphereSubzionToHexObj(obj3, nx4, *pobj1);
		sphereSubzionToHexObj(obj4, nx4, *pobj2);
		MergeHexObj(*pobj1, *pobj2, tmpObj2);
		delete pobj1;
		delete pobj2;
	}
	
	CHexObj *pQuarterObj = new CHexObj;
	assert(pQuarterObj!=NULL);
	MergeHexObj(tmpObj1, tmpObj2, *pQuarterObj);
	return pQuarterObj;
}


//====================================================================
int CMeshTetraDialog::m_nX = 4;
int CMeshTetraDialog::m_nY = 4;
int CMeshTetraDialog::m_nZ = 4;
Vector3d CMeshTetraDialog::m_vVertex[4]={
	Vector3d(0.5f, -0.5f, 0), 
	Vector3d(0.0f, +0.5f, 0), 
	Vector3d(-0.5f, -0.5f, 0), 
	Vector3d(0, 0, 0.75f)}; 


CMeshTetraDialog::CMeshTetraDialog(QWidget* parent, const char* name):
	CMeshPrimitiveDialog(parent, name, false, Qt::WStyle_Tool)
{  
	setCaption(name);
	QValueList<int> slist;
	slist.append(80);
	slist.append(200);
	m_pSplitter->setSizes(slist);

	//generate the symbol table
	prepareNameTable();
	appendToListview();
	m_pGLPreviewWin->setNameTable(m_table, m_nLen, 0);  //0: brick type
	m_pGLPreviewWin->camera()->frame()->setSpinningSensitivity(10);

	//fix the window size
	QSize sz =this->size();
	setFixedSize(sz);
}


CMeshTetraDialog::~CMeshTetraDialog(void)
{
}


void CMeshTetraDialog::prepareNameTable( void )
{
	int & i = m_nLen;
	i=0;
	m_table[i++]=CNameTableItem("Node 1", 'V', &m_vVertex[0]);
	m_table[i++]=CNameTableItem("Node 2", 'V', &m_vVertex[1]);
	m_table[i++]=CNameTableItem("Node 3", 'V', &m_vVertex[2]);
	m_table[i++]=CNameTableItem("Node 4", 'V', &m_vVertex[3]);	
	m_table[i++]=CNameTableItem("Division #", 'i', &m_nX);
	//m_table[i++]=CNameTableItem("Div Y", 'i', &m_nY);
	//m_table[i++]=CNameTableItem("Div Z", 'i', &m_nZ);
}


void CMeshTetraDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Create a sphere object
	CHexObj *pobj = meshTetrahedron(m_nX, m_nX, m_nX, m_vVertex);
	if (pobj==NULL) return;

    AxisAlignedBox box;
	pobj->ComputeBoundingBox(box.minp, box.maxp);
	pobj->SetBoundingBox(box);
	char namebuff[256];
	sprintf(namebuff, "Tetrahedron%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  	
}

/*
void CMeshTetraDialog::onApply(void)
{
	//Delete old object
	if (m_pGLPreviewWin->m_pObj) 
		delete m_pGLPreviewWin->m_pObj;
	m_pGLPreviewWin->m_pObj = NULL;

	//Create a sphere object
	Vector3f* pVertex=NULL;
	Vector4i* pQuad=NULL;
	Vector8i* pHex=NULL;
	int nVertexCount=0, nPolygonCount=0, nHexCount=0;
	meshSphereVertices(m_vCenter, m_fR, 
			m_fA1, m_fA2, m_fB1, m_fB2,
			1, m_nArcDiv, 
			pVertex, nVertexCount);

	int nx, ny, nz;
	//getBrickElements(nx, ny, nz, pHex, nHexCount);
	//getBrickBounday(nx, ny, nz, pQuad, nPolygonCount);

	CHexObj *p = new CHexObj(pVertex, nVertexCount, pHex, nHexCount, pQuad, nPolygonCount);
	if (p==NULL) return;
	Vector3f lowleft=m_vCenter-Vector3f(m_fR); 
	Vector3f upright=m_vCenter+Vector3f(m_fR); 
	p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
	char namebuff[256];
	sprintf(namebuff, "Brick%d", 0);
	p->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(p);
	m_pGLPreviewWin->updateGL();  	
}
*/