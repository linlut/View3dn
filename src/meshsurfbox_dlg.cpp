//FILE: meshsphereshelldialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshsurfbox_dlg.h"
#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


//=======================================================================
//in file mesh2drect_dlg.cpp
extern CQuadObj *
createRectSurf(const Vector3d& lowleft, const Vector3d& upright, const int nx, const int ny, const int nz);

static CQuadObj *makeBoxSurface(const Vector3d& lowleft, const Vector3d& upright, 
                                 const int nx, const int ny, const int nz) 
{
    Vector3d v1, v2;
    CQuadObj *pobj1, *pobj2, obj3;

    {
        v1=lowleft, v2=upright;
        v2.z = v1.z;
        CQuadObj *o1 = createRectSurf(v1, v2, nx, ny, 0);
        CQuadObj o2(*o1);
        o2.applyTranslation(0, 0, upright.z-lowleft.z);
        o1->flipNormal();
        pobj1 = new CQuadObj;
	    MergeQuadObj(*o1, o2, *pobj1);
        delete o1;
    }
    {
        v1=lowleft, v2=upright;
        v2.x = v1.x;
        CQuadObj *o1 = createRectSurf(v1, v2, 0, ny, nz);
        CQuadObj o2(*o1);
        o2.applyTranslation(upright.x-lowleft.x,0,0);
        o1->flipNormal();
        pobj2 = new CQuadObj;
	    MergeQuadObj(*o1, o2, *pobj2);
        delete o1;
    }
	MergeQuadObj(*pobj1, *pobj2, obj3);
    delete pobj1;
    delete pobj2;

    {
        v1=lowleft, v2=upright;
        v2.y = v1.y;
        CQuadObj *o1 = createRectSurf(v1, v2, nx, 0, nz);
        CQuadObj o2(*o1);
        o2.applyTranslation(0, upright.y-lowleft.y,0);
        o2.flipNormal();
        pobj2 = new CQuadObj;
	    MergeQuadObj(*o1, o2, *pobj2);
        delete o1;
    }

    pobj1 = new CQuadObj;
    MergeQuadObj(*pobj2, obj3, *pobj1);
    delete pobj2;

    return pobj1;
}

//====================================================================
int CMeshSurfBoxDlg::m_nx = 5;
int CMeshSurfBoxDlg::m_ny = 4;
int CMeshSurfBoxDlg::m_nz = 3;
Vector3d CMeshSurfBoxDlg::m_vLowleft = Vector3d(-0.5,-0.5,-0.3);
Vector3d CMeshSurfBoxDlg::m_vUpright = Vector3d(0.5,0.5,0.3);
Vector3d CMeshSurfBoxDlg::m_vCenter = Vector3d(0,0,0);


CMeshSurfBoxDlg::CMeshSurfBoxDlg(QWidget* parent, const char* name):
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


CMeshSurfBoxDlg::~CMeshSurfBoxDlg(void)
{
}


void CMeshSurfBoxDlg::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Lowleft", 'V', &m_vLowleft);
	m_table[i++]=CNameTableItem("Upright", 'V', &m_vUpright);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("nx #", 'i', &m_nx);
	m_table[i++]=CNameTableItem("ny #", 'i', &m_ny);
	m_table[i++]=CNameTableItem("nz #", 'i', &m_nz);
}


void CMeshSurfBoxDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_nx<2) m_nx=2;
	if (m_ny<2) m_ny=2;
	if (m_nz<2) m_nz=2;

	//Create the first quater cyl object
	CQuadObj *pobj=makeBoxSurface(m_vLowleft, m_vUpright, m_nx, m_ny, m_nz); 

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