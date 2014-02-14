//FILE: meshsurfcone_dlg.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"
#include "meshsurfcone_dlg.h"


//=======================================================================
static void 
coneSubzionToQuadObj(const int zid, MV3& vertex, const int nx, CQuadObj &obj)
{ 
	int _tmp, ny, nz, c, x1, x2, y1, y2, z1, z2, nsize, NX, NY;
	vertex.getMatrixDimension(_tmp, ny, nz);
	assert(_tmp>=nx);

    switch(zid){
    case 2:
    case 3:
        x1=nx, x2=nx;
        y1=1, y2=ny;
        z1=1, z2=nz;
        nsize = ny*nz;
        NX=ny, NY=nz;
        break;
    case 4:
        x1=1, x2=nx;
        y1=1, y2=ny;
        z1=nz, z2=nz;
        nsize=nx*ny;
        NX=nx, NY=ny;
        break;
    default:
        assert(0);
    }

	//copy the first layer vertices
	Vector3d *meshvert = new Vector3d[nsize];
	assert(meshvert!=NULL);
	c = 0;
	for (int k=z1; k<=z2; k++){
		for (int j=y1; j<=y2; j++){
			for (int i=x1; i<=x2; i++, c++)
				meshvert[c] = vertex(i, j, k);
		}
	}
	obj.m_nVertexCount = nsize;
	obj.m_pVertex = meshvert;
	Vector4i * &m_pQuad = (Vector4i * &)obj.m_pPolygon;
    getRectangleSurface(NX-1, NY-1, m_pQuad, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
}


static CQuadObj* meshQuarterCone(
	const FREAL& r, const FREAL& h, const FREAL& a0, const FREAL& a1, 
	const int nsplitw, const int nsplith)
{
	MV3 vertex;
	M1 alpha(2);

	//create the sphere elements
	alpha(1)=a0, alpha(2)=a1;
	coneMesher(nsplitw, nsplith, alpha, r, h, vertex);

	//separate the 3 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx4 = nx/4;
	const int nsize = nx4*ny*nz;
	assert(nx%4==0);
	MV3 obj2=vertex(1+nx4);
	MV3 obj3=vertex(1+nx4*2);
	MV3 obj4=vertex(1+nx4*3);
	CQuadObj *pQuarterObj = new CQuadObj;
	assert(pQuarterObj!=NULL);

	CQuadObj tmpObj1;
	{
        CQuadObj tmpObj2;
		CQuadObj *pobj1 = new CQuadObj;
		assert(pobj1!=NULL);
		coneSubzionToQuadObj(2, obj2, nx4, *pobj1);
		coneSubzionToQuadObj(3, obj3, nx4, tmpObj2);
		MergeQuadObj(*pobj1, tmpObj2, tmpObj1);
		delete pobj1;
	}
	{
		CQuadObj *pobj1 = new CQuadObj;
		assert(pobj1!=NULL);
		coneSubzionToQuadObj(4, obj4, nx4, *pobj1);
		MergeQuadObj(*pobj1, tmpObj1, *pQuarterObj);
		delete pobj1;
	}
	
	return pQuarterObj;
}


//====================================================================
int CMeshSurfConeDlg::m_nArcDiv = 4;
int CMeshSurfConeDlg::m_nHgtDiv = 4;
double CMeshSurfConeDlg::m_fR =  0.8;
double CMeshSurfConeDlg::m_fH =  1.0;
double CMeshSurfConeDlg::m_fA1 = 0;
double CMeshSurfConeDlg::m_fA2 = 90;
Vector3d CMeshSurfConeDlg::m_vCenter = Vector3d(0,0,0);


CMeshSurfConeDlg::CMeshSurfConeDlg(QWidget* parent, const char* name):
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


CMeshSurfConeDlg::~CMeshSurfConeDlg(void)
{
}


void CMeshSurfConeDlg::prepareNameTable( void )
{
	int & i = m_nLen;
	i=0;
	m_table[i++]=CNameTableItem("Radius", 'd', &m_fR);
	m_table[i++]=CNameTableItem("Height", 'd', &m_fH);
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Div #", 'i', &m_nArcDiv);
}


void CMeshSurfConeDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check parameters
	if (m_fR<=0 || m_fH<=0) return;
	if (m_nArcDiv<1) m_nArcDiv=1;
	if (m_nHgtDiv<1) m_nHgtDiv=1;
	int nzone;
	FREAL zoneangle;
	GET_ANGULAR_SECTIONS(m_fA1, m_fA2, 360, nzone, zoneangle);
	if (nzone==0) return;
	int nArcDiv = m_nArcDiv/nzone;
	if (nArcDiv<1) nArcDiv=1;

	//Create the first quater cyl object
	CQuadObj *pobj;
	const FREAL a1=m_fA1;
	const FREAL a2=m_fA1+zoneangle;
	CQuadObj *p1 = meshQuarterCone(m_fR, m_fH, a1, a2, nArcDiv, nArcDiv);
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
	sprintf(namebuff, "Cone%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  	
}
