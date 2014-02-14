//FILE: meshsphereshelldialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshsurfcyl_dlg.h"
#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


//=======================================================================

static Vector3d* extractContourLine(MV3& vertex, const int nHgtDiv, int &NX)
{ 
	int nx, ny, nz, i;
	vertex.getMatrixDimension(nx, ny, nz);

    NX = nx-1;
	const int nsize = NX*(nHgtDiv+1);
	Vector3d *meshvert = new Vector3d[nsize];
	assert(meshvert!=NULL);

    int c = 0;
    int midx = nx/2;
    for (i=1; i<=midx; i++, c++)
        meshvert[c]=vertex(i, 1, 1);
    for (i=midx+2; i<=nx; i++, c++)
        meshvert[c]=vertex(i, 1, 1);
    return meshvert;
}


static CQuadObj* meshQuarterCylinderShell(
	const FREAL& r, const FREAL &h, const FREAL& a0, const FREAL& a1, 
	const int nsplitw, const int nsplith)
{
	MV3 vertex;
	M1 alpha(2);

	//create the sphere elements
    const FREAL th=0.1*r;
    const int nsplitt=1;
	alpha(1)=a0, alpha(2)=a1;
	cylinderShell(nsplitw, nsplitt, 1, r, 0, th, a0, a1, vertex);

	//separate the 2 zones
    int c, nx;
    Vector3d* meshvert=extractContourLine(vertex, nsplith, nx);
    assert(meshvert!=NULL);

    c = nx;
    for (int j=1; j<=nsplith; j++){
        const FREAL z = h/nsplith*((double)j);
        for (int i=0; i<nx; i++, c++){
			meshvert[c] = meshvert[i];
            meshvert[c].z = z;
		}
	}

    CQuadObj *pobj = new CQuadObj; assert(pobj!=NULL);
    CQuadObj &obj=*pobj;
	obj.m_nVertexCount = nx*(nsplith+1);
	obj.m_pVertex = meshvert;
	Vector4i * &m_pQuad = (Vector4i * &)obj.m_pPolygon;
    getRectangleSurface(nx-1, nsplith, m_pQuad, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
    return pobj;
}



//====================================================================
int CMeshSurfCylinderDlg::m_nArcDiv = 8;
int CMeshSurfCylinderDlg::m_nHgtDiv = 4;
double CMeshSurfCylinderDlg::m_fR =  0.5000;
double CMeshSurfCylinderDlg::m_fHeight = 1.000;
double CMeshSurfCylinderDlg::m_fA1 = 0;
double CMeshSurfCylinderDlg::m_fA2 = 90;
Vector3d CMeshSurfCylinderDlg::m_vCenter = Vector3d(0,0,0);


CMeshSurfCylinderDlg::CMeshSurfCylinderDlg(QWidget* parent, const char* name):
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


CMeshSurfCylinderDlg::~CMeshSurfCylinderDlg(void)
{
}


void CMeshSurfCylinderDlg::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Outer Radius", 'd', &m_fR);
	m_table[i++]=CNameTableItem("Height", 'd', &m_fHeight);
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Div #", 'i', &m_nArcDiv);
	m_table[i++]=CNameTableItem("Height Div #", 'i', &m_nHgtDiv);
}


void CMeshSurfCylinderDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_fR<=0) return;
	if (m_fHeight<=0) return;
	if (m_nArcDiv<2) m_nArcDiv=2;
	if (m_nHgtDiv<1) m_nHgtDiv=1;
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
	CQuadObj *p1 = meshQuarterCylinderShell(m_fR, m_fHeight, a1, a2, nArcDiv, m_nHgtDiv);

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