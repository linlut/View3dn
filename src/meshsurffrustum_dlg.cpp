//FILE: meshsphereshelldialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshsurffrustum_dlg.h"
#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


//=======================================================================

static Vector3d* extractContourLines(MV3& vertex1, MV3& vertex2, const int nHgtDiv, int &NX)
{ 
	int nx, ny, nz, i;
	vertex1.getMatrixDimension(nx, ny, nz);

    NX = nx-1;
	const int nsize = NX*(nHgtDiv+1);
	Vector3d *meshvert = new Vector3d[nsize];
	assert(meshvert!=NULL);

    int midx = nx/2;
    int c = 0;
    for (i=1; i<=midx; i++, c++)
        meshvert[c]=vertex1(i, 1, 1);
    for (i=midx+2; i<=nx; i++, c++)
        meshvert[c]=vertex1(i, 1, 1);

    c = NX*nHgtDiv;
    for (i=1; i<=midx; i++, c++)
        meshvert[c]=vertex2(i, 1, 1);
    for (i=midx+2; i<=nx; i++, c++)
        meshvert[c]=vertex2(i, 1, 1);
    return meshvert;
}


static CQuadObj* meshQuarterCylinderShell(
	const FREAL& r1, const FREAL& r2, const FREAL &h, const FREAL& a0, const FREAL& a1, 
	const int nsplitw, const int nsplith)
{
	MV3 vertex1, vertex2;
	M1 alpha(2);

	//create the sphere elements
    const FREAL th=0;
    const FREAL _H = 0;
    const int nsplitt=1;
	alpha(1)=a0, alpha(2)=a1;
	cylinderShell(nsplitw, nsplitt, 1, r1, _H, th, a0, a1, vertex1);
	cylinderShell(nsplitw, nsplitt, 1, r2, _H, th, a0, a1, vertex2);

	//separate the 2 zones
    int c, nx;
    Vector3d* meshvert=extractContourLines(vertex1, vertex2, nsplith, nx);
    assert(meshvert!=NULL);

    c = nx;
    for (int j=1; j<=nsplith; j++){
        Vector3d *p1=meshvert;
        Vector3d *p2=meshvert+nx*nsplith;
        const FREAL t = (double)j/(double)nsplith;
        const FREAL t1=1.0-t;
        const FREAL z = h*t;
        for (int i=0; i<nx; i++, c++, p1++, p2++){
            Vector3d *pv = &meshvert[c];
			pv->x = p1->x*t1+p2->x*t;
			pv->y = p1->y*t1+p2->y*t;
            pv->z = z;
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
int CMeshSurfFrustumDlg::m_nArcDiv = 8;
int CMeshSurfFrustumDlg::m_nHgtDiv = 4;
double CMeshSurfFrustumDlg::m_fR1 =  1;
double CMeshSurfFrustumDlg::m_fR2 =  0.50;
double CMeshSurfFrustumDlg::m_fHeight = 1;
double CMeshSurfFrustumDlg::m_fA1 = 0;
double CMeshSurfFrustumDlg::m_fA2 = 90;
Vector3d CMeshSurfFrustumDlg::m_vCenter = Vector3d(0,0,0);


CMeshSurfFrustumDlg::CMeshSurfFrustumDlg(QWidget* parent, const char* name):
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


CMeshSurfFrustumDlg::~CMeshSurfFrustumDlg(void)
{
}


void CMeshSurfFrustumDlg::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Bottom Radius", 'd', &m_fR1);
	m_table[i++]=CNameTableItem("Top Radius", 'd', &m_fR2);
	m_table[i++]=CNameTableItem("Height", 'd', &m_fHeight);
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Div #", 'i', &m_nArcDiv);
	m_table[i++]=CNameTableItem("Height Div #", 'i', &m_nHgtDiv);
}


void CMeshSurfFrustumDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_fR1<=0) return;
	if (m_fR2<=0) return;
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
	CQuadObj *p1 = meshQuarterCylinderShell(m_fR1, m_fR2, m_fHeight, a1, a2, nArcDiv, m_nHgtDiv);

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