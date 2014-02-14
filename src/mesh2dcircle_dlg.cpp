//FILE: meshspheredialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "mesh2dcircle_dlg.h"
#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


//=======================================================================
void circle2dSubzoneToQuadObj(MV3& vertex, const int nx, CQuadObj &obj)
{ 
	int _tmp, ny, nz, c, i, j;
	vertex.getMatrixDimension(_tmp, ny, nz);
	nz=1;
	assert(_tmp>=nx);
	const int nsize = nx*ny*nz;
	const int nv = nsize;
	Vector3d *meshvert = new Vector3d[nv];
	assert(meshvert!=NULL);

	//copy the first layer vertices
	c = 0;
	for (int k=1; k<=nz; k++){
		for (j=1; j<=ny; j++){
			for (i=1; i<=nx; i++, c++)
				meshvert[c] = vertex(i, j, k);
		}
	}
	obj.m_nVertexCount = nv;
	obj.m_pVertex = meshvert;
	Vector4i * &pQuadO = (Vector4i * &)obj.m_pPolygon;
    getRectangleSurface(nx-1, ny-1, pQuadO, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
}


static CQuadObj* 
meshQuarterCylinder(const FREAL& r, const FREAL& a0, const FREAL& a1, const int nsplitw)
{
	MV3 vertex;
	M1 alpha(2);

	//create the sphere elements
    FREAL h=0;
	alpha(1)=a0, alpha(2)=a1;
	colmshzn(nsplitw+1, r, alpha, h, vertex);

	//separate the 3 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	assert(nz==1);
	const int nx3 = nx/3;
	assert(nx%3==0);
	MV3 cyl1=vertex(1);
	MV3 cyl2=vertex(1+nx3);
	MV3 cyl3=vertex(1+nx3+nx3);
	CQuadObj tmpObj;
    CQuadObj *pQuarterCyl=new CQuadObj; assert(pQuarterCyl!=NULL);

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

int CMesh2DCircleDlg::m_nDivArc=4;
double CMesh2DCircleDlg::m_fR =  0.5;
double CMesh2DCircleDlg::m_fA1 = 0;
double CMesh2DCircleDlg::m_fA2 = 90;
Vector3d CMesh2DCircleDlg::m_vCenter = Vector3d(0,0,0);


CMesh2DCircleDlg::CMesh2DCircleDlg(QWidget* parent, const char* name):
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
    //setSizeGripEnabled( false );
	//draw the default model
	//onApply();
}


void CMesh2DCircleDlg::resize(int w, int h)
{
    return;
    QSize sz= m_pSplitter->size();
    QSize szwin= this->size();
    const int dx = szwin.width()-sz.width();
    const int dy = szwin.height()-sz.height();

    CMeshPrimitiveDialog::resize(w, h);
	m_pSplitter->resize(w-dx, h-dy);
}


CMesh2DCircleDlg::~CMesh2DCircleDlg(void)
{
}


void CMesh2DCircleDlg::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Radius", 'd', &m_fR);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Division", 'i', &m_nDivArc);
}



void CMesh2DCircleDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check parameters
	if (m_fR<=0) return;
	if (m_nDivArc<1) m_nDivArc=1;
	int nzone;
	FREAL zoneangle;
	GET_ANGULAR_SECTIONS(m_fA1, m_fA2, 360, nzone, zoneangle);
	if (nzone==0) return;
	int nDivArc = m_nDivArc/nzone;
	if (nDivArc<1) nDivArc=1;

	//Create the first quater cyl object
	CQuadObj *pobj;
	CQuadObj* p1=meshQuarterCylinder(m_fR, m_fA1, m_fA1+zoneangle, nDivArc);
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
	sprintf(namebuff, "Cylinder%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  	
}


