//FILE: mesh2dcircleshell_dlg.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "mesh2dcircleshell_dlg.h"
#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


//=======================================================================
//in mesh2dcircle_dlg.cpp
extern void circle2dSubzoneToQuadObj(MV3& vertex, const int nx, CQuadObj &obj);


static CQuadObj* meshQuarterCylinderShell(
	const FREAL& r, const FREAL &th, const FREAL& a0, const FREAL& a1, 
	const int nsplitw, const int nsplitt)
{
	MV3 vertex;
	M1 alpha(2);

	//create the sphere elements
	alpha(1)=a0, alpha(2)=a1;
    const FREAL h=0;
	cylinderShell(nsplitw, nsplitt, 1, r, h, th, a0, a1, vertex);

	//separate the 2 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx3 = nx/2;
	MV3 cyl1=vertex(1);
	MV3 cyl2=vertex(1+nx3);
	CQuadObj _tmpObj1, _tmpObj2;
    CQuadObj *pQuarterCyl=new CQuadObj; assert(pQuarterCyl!=NULL);
	circle2dSubzoneToQuadObj(cyl1, nx3, _tmpObj1); 
	circle2dSubzoneToQuadObj(cyl2, nx3, _tmpObj2);
	MergeQuadObj(_tmpObj1, _tmpObj2, *pQuarterCyl);
	return pQuarterCyl;
}



//====================================================================
int CMesh2DCircleShellDlg::m_nArcDiv = 8;
int CMesh2DCircleShellDlg::m_nThkDiv = 2;
double CMesh2DCircleShellDlg::m_fR =  0.5000;
double CMesh2DCircleShellDlg::m_fThickness = 0.1000;
double CMesh2DCircleShellDlg::m_fA1 = 0;
double CMesh2DCircleShellDlg::m_fA2 = 90;
Vector3d CMesh2DCircleShellDlg::m_vCenter = Vector3d(0,0,0);


CMesh2DCircleShellDlg::CMesh2DCircleShellDlg(QWidget* parent, const char* name):
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


CMesh2DCircleShellDlg::~CMesh2DCircleShellDlg(void)
{
}


void CMesh2DCircleShellDlg::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Outer Radius", 'd', &m_fR);
	m_table[i++]=CNameTableItem("Shell Thickness", 'd', &m_fThickness);
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Div #", 'i', &m_nArcDiv);
	m_table[i++]=CNameTableItem("Thick Div #", 'i', &m_nThkDiv);
}


void CMesh2DCircleShellDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_fR<=0) return;
	if (m_fThickness<=0) return;
	if (m_nArcDiv<2) m_nArcDiv=2;
	if (m_nThkDiv<1) m_nThkDiv=1;
	int nzone;
	FREAL zoneangle;
	GET_ANGULAR_SECTIONS(m_fA1, m_fA2, 360, nzone, zoneangle);
	if (nzone==0) return;
	int nArcDiv = m_nArcDiv/nzone;
	if (nArcDiv<2) nArcDiv=2;

	//Create the first quater cyl object
	CQuadObj *pobj;
	const FREAL a1=m_fA1;
	const FREAL a2=m_fA1+zoneangle;
	CQuadObj *p1 = meshQuarterCylinderShell(m_fR, m_fThickness, a1, a2, nArcDiv, m_nThkDiv);

	if (p1==NULL) return;
	pobj = p1;
	for (int i=1; i<nzone; i++){
		CQuadObj *pobj0=pobj;
		CQuadObj *pdup = new CQuadObj(*p1);
		pobj = new CQuadObj;
		FAST_ROTATE_Z_AXIS(pdup->m_pVertex, pdup->m_nVertexCount, zoneangle);
		MergeQuadObj(*pobj0, *pdup, *pobj);
		delete p1;
		p1 = pdup;
	}
	if (pobj!=p1) delete p1;

	//Dont forget to translate m_vCenter;
	if (!(m_vCenter==Vector3d(0,0,0))){
		const double tx = m_vCenter.x;
		const double ty = m_vCenter.y;
		const double tz = m_vCenter.z;		
		pobj->applyTranslation(tx, ty, tz);
	}

	//Set bounding box
    AxisAlignedBox box;
	pobj->ComputeBoundingBox(box.minp, box.maxp);
	pobj->SetBoundingBox(box);
	char namebuff[256];
	sprintf(namebuff, "CylinderShell%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  
}