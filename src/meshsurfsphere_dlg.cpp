//FILE: meshsurfsphere_dlg.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshsurfsphere_dlg.h"
#include "qmeshobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"



//=======================================================================
static void 
sphereSubzoneToQuadObj(const int zone, MV3& vertex, const int nx, CQuadObj &obj)
{ 
	int nsize, _tmp, ny, nz, c=0;
	vertex.getMatrixDimension(_tmp, ny, nz);
	assert(_tmp>=nx);

	//copy the outer layer vertices
    int x1, x2, y1, y2, z1, z2;
    switch(zone){
    case 1:
    case 2:
    case 3:
        x1=1, x2=nx;
        y1=1, y2=ny;
        z1=nz, z2=nz;
	    nsize = nx*ny;
        break;
    default:
        assert(0);
    }

	Vector3d *meshvert = new Vector3d[nsize];
	assert(meshvert!=NULL);
	for (int k=z1; k<=z2; k++){
		for (int j=y1; j<=y2; j++){
			for (int i=x1; i<=x2; i++, c++)
				meshvert[c] = vertex(i, j, k);
		}
	}
	obj.m_nVertexCount = nsize;
	obj.m_pVertex = meshvert;
	Vector4i * &m_pQuad = (Vector4i * &)obj.m_pPolygon;
    getRectangleSurface(nx-1, ny-1, m_pQuad, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
}


static CQuadObj* meshQuarterSphere(
	const FREAL& r, const FREAL& a0, const FREAL& a1, 
	const FREAL& b0, const FREAL& b1, const int nsplit)
{
	MV3 vertex;
	M1 alpha(2), beta(2);

	//create the sphere elements
	alpha(1)=a0, alpha(2)=a1;
	beta(1)=b0, beta(2)=b1;
    const int nsplith=1;
    const FREAL thk=0;
    shereShell(nsplit, nsplith, r, thk, a0, a1, b0, b1, vertex);

	//separate the 3 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx3 = nx/3;
	assert(nx%3==0);
	MV3 obj1=vertex(1);
	MV3 obj2=vertex(1+nx3);
	MV3 obj3=vertex(1+nx3*2);

	CQuadObj tmpObj1;
	CQuadObj *pQuarterObj = new CQuadObj;
	assert(pQuarterObj!=NULL);
	{
		CQuadObj *pobj1 = new CQuadObj;
	    CQuadObj tmpObj2;
		assert(pobj1!=NULL);
		sphereSubzoneToQuadObj(1, obj1, nx3, *pobj1);
		sphereSubzoneToQuadObj(2, obj2, nx3, tmpObj2);
		MergeQuadObj(*pobj1, tmpObj2, tmpObj1);
		delete pobj1;
	}
	{
	    CQuadObj tmpObj2;
		sphereSubzoneToQuadObj(3, obj3, nx3, tmpObj2);
		MergeQuadObj(tmpObj1, tmpObj2, *pQuarterObj);
	}
	
	return pQuarterObj;
}


//====================================================================
int CMeshSurfSphereDlg::m_nArcDiv = 2;
double CMeshSurfSphereDlg::m_fR =  0.5f;
double CMeshSurfSphereDlg::m_fA1 = 0;
double CMeshSurfSphereDlg::m_fA2 = 90;
double CMeshSurfSphereDlg::m_fB1 = 0;
double CMeshSurfSphereDlg::m_fB2 = 90;
Vector3d CMeshSurfSphereDlg::m_vCenter = Vector3d(0,0,0);


CMeshSurfSphereDlg::CMeshSurfSphereDlg(QWidget* parent, const char* name):
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


CMeshSurfSphereDlg::~CMeshSurfSphereDlg(void)
{
}


void CMeshSurfSphereDlg::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Radius", 'd', &m_fR);
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Beta1", 'd', &m_fB1);
	m_table[i++]=CNameTableItem("Beta2", 'd', &m_fB2);

	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Div #", 'i', &m_nArcDiv);
}


void CMeshSurfSphereDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_fR<=0) return;
	if (m_nArcDiv<2) m_nArcDiv=2;
	int nzone1, nzone2;
	FREAL zoneangle1, zoneangle2;
	GET_ANGULAR_SECTIONS(m_fA1, m_fA2, 360, nzone1, zoneangle1);
	GET_ANGULAR_SECTIONS(m_fB1, m_fB2, 180, nzone2, zoneangle2);
	zoneangle2 = 90;
	if (nzone1==0 || nzone2==0) return;
	int nArcDiv=m_nArcDiv/nzone1; 
	if (nArcDiv<2) nArcDiv=2;

	//Create the first quater cyl object
	CQuadObj *pobj;
	const FREAL a1=m_fA1;
	const FREAL a2=a1+zoneangle1;
	FREAL b1;
	if (m_fB1<90) b1=0;
	else b1=90;
	const FREAL b2=b1+zoneangle2;
	CQuadObj *p1;
	if (b1==0)
		p1=meshQuarterSphere(m_fR, a1, a2, b1, b2, nArcDiv);
	else{
		p1=meshQuarterSphere(m_fR, a1, a2, 0, 90, nArcDiv);
		p1->mirrorXYPlane();
	}
	if (nzone2==2){
		CQuadObj *p2 = new CQuadObj(*p1);
		CQuadObj *p3 = new CQuadObj;
		assert(p2!=NULL && p3!=NULL);
		p2->mirrorXYPlane();
		MergeQuadObj(*p1, *p2, *p3);
		delete p1; delete p2;
		p1 = p3;
	}

	//Make copies of the first part
	if (p1==NULL) return;
	pobj = p1;
	for (int i=1; i<nzone1; i++){
		CQuadObj *pobj0=pobj;
		CQuadObj *pdup = new CQuadObj(*p1);
		pobj = new CQuadObj;
		FAST_ROTATE_Z_AXIS(pdup->m_pVertex, pdup->m_nVertexCount, zoneangle1);
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
	sprintf(namebuff, "Sphere%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  
	
}