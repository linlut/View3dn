//FILE: mesh2dtri_dlg.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "mesh2dtri_dlg.h"
#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"



//=======================================================================
//in mesh2dcircle_dlg.cpp
extern void circle2dSubzoneToQuadObj(MV3& vertex, const int nx, CQuadObj &obj);

static CQuadObj* 
mesh2dTriangle(const Vector3d& v1, const Vector3d& v2, const Vector3d& v3, const int nsplitw)
{
	MV3 vertex;
	M2 xyzo(3,6);
    extern void triangleMesher(const Vector3d& v1, const Vector3d& v2, const Vector3d& v3, const int nArcDiv, MV3& vertex);

	//create the sphere elements
	triangleMesher(v1, v2, v3, nsplitw, vertex);

	//separate the 3 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx3 = nx/3;
	const int nsize = nx3*ny*nz;
	assert(nx%3==0);
	MV3 cyl1=vertex(1);
	MV3 cyl2=vertex(1+nx3);
	MV3 cyl3=vertex(1+nx3+nx3);
	CQuadObj tmpObj, *pQuarterCyl=new CQuadObj;
	assert(pQuarterCyl!=NULL);
	{
		CQuadObj *obj1 = new CQuadObj;
		CQuadObj *obj2 = new CQuadObj;
		assert(obj1!=NULL && obj2!=NULL);
		circle2dSubzoneToQuadObj(cyl1, nx3, *obj1);
		circle2dSubzoneToQuadObj(cyl2, nx3, *obj2);
		MergeQuadObj(*obj1, *obj2, tmpObj);
		delete obj1;
		delete obj2;
	}
	{
		CQuadObj *obj3= new CQuadObj;
		assert(obj3!=NULL);
		circle2dSubzoneToQuadObj(cyl3, nx3, *obj3);
		MergeQuadObj(tmpObj, *obj3, *pQuarterCyl);
		delete obj3;
	}

	return pQuarterCyl;	
}


//========================================================================

Vector3d CMesh2DTriangleDlg::m_vVertex1 = Vector3d(0.5, -0.25, 0);
Vector3d CMesh2DTriangleDlg::m_vVertex2 = Vector3d(0, 0.5, 0);
Vector3d CMesh2DTriangleDlg::m_vVertex3 = Vector3d(-0.5, -0.25, 0);
Vector3d CMesh2DTriangleDlg::m_vTranslate = Vector3d(0,0,0);
int CMesh2DTriangleDlg::m_nArcDiv=4;


CMesh2DTriangleDlg::CMesh2DTriangleDlg(QWidget* parent, const char* name):
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

	//draw the default model
	//onApply();
}


CMesh2DTriangleDlg::~CMesh2DTriangleDlg(void)
{
}


void CMesh2DTriangleDlg::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("x1", 'd', &m_vVertex1.x);
	m_table[i++]=CNameTableItem("y1", 'd', &m_vVertex1.y);
	m_table[i++]=CNameTableItem("x2", 'd', &m_vVertex2.x);
	m_table[i++]=CNameTableItem("y2", 'd', &m_vVertex2.y);
	m_table[i++]=CNameTableItem("x3", 'd', &m_vVertex3.x);
	m_table[i++]=CNameTableItem("y3", 'd', &m_vVertex3.y);

	m_table[i++]=CNameTableItem("Translate", 'V', &m_vTranslate);
	m_table[i++]=CNameTableItem("Arc Division", 'i', &m_nArcDiv);
}



void CMesh2DTriangleDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();
	if (m_nArcDiv<1) m_nArcDiv=1;

	//Create the first quater cyl object
	CQuadObj *pobj;
	CQuadObj* p1=mesh2dTriangle(m_vVertex1, m_vVertex2, m_vVertex3, m_nArcDiv);
	if (p1==NULL) return;
	pobj = p1;

	//Dont forget to translate m_vCenter;
	if (!(m_vTranslate==Vector3d(0,0,0))){
		const double tx = m_vTranslate.x;
		const double ty = m_vTranslate.y;
		const double tz = m_vTranslate.z;		
		pobj->applyTranslation(tx, ty, tz);
	}

	//Set bounding box
    AxisAlignedBox box;
	pobj->ComputeBoundingBox(box.minp, box.maxp);
	pobj->SetBoundingBox(box);
	char namebuff[256];
	sprintf(namebuff, "Cylinder%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  	
}


