//FILE: meshsphereshelldialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshsphereshelldialog.h"
#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


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


static CHexObj* meshQuarterSphereShell(
	const FREAL& r1, const FREAL &r2, const FREAL& a0, const FREAL& a1, 
	const FREAL& b0, const FREAL& b1, const int nsplitw, const int nsplith)
{
	MV3 vertex;

	//create the sphere elements
	//	sprmshzn(nsplit, alpha, beta, r1, vertex);
	shereShell(nsplitw, nsplith, r1, r2, a0, a1, b0, b1, vertex);

	//separate the 3 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx3 = nx/3;
	const int nsize = nx3*ny*nz;
	assert(nx%3==0);
	MV3 obj1=vertex(1);
	MV3 obj2=vertex(1+nx3);
	MV3 obj3=vertex(1+nx3*2);
	CHexObj *pQuarterObj = new CHexObj;
	assert(pQuarterObj!=NULL);

	CHexObj tmpObj1;
	{
		CHexObj *pobj1 = new CHexObj;
		CHexObj *pobj2 = new CHexObj;
		assert(pobj1!=NULL && pobj2!=NULL);
		sphereSubzionToHexObj(obj1, nx3, *pobj1);
		sphereSubzionToHexObj(obj2, nx3, *pobj2);
		MergeHexObj(*pobj1, *pobj2, tmpObj1);
		delete pobj1;
		delete pobj2;
	}
	{
		CHexObj *pobj1 = new CHexObj;
		assert(pobj1!=NULL);
		sphereSubzionToHexObj(obj3, nx3, *pobj1);
		MergeHexObj(*pobj1, tmpObj1, *pQuarterObj);
		delete pobj1;
	}
	return pQuarterObj;
}


//====================================================================
int CMeshSphereShellDialog::m_nArcDiv = 4;
int CMeshSphereShellDialog::m_nHgtDiv = 1;
double CMeshSphereShellDialog::m_fR =  0.50000;
double CMeshSphereShellDialog::m_fH =  0.10000;
double CMeshSphereShellDialog::m_fA1 = 0;
double CMeshSphereShellDialog::m_fA2 = 90;
double CMeshSphereShellDialog::m_fB1 = 0;
double CMeshSphereShellDialog::m_fB2 = 90;
Vector3d CMeshSphereShellDialog::m_vCenter = Vector3d(0,0,0);


CMeshSphereShellDialog::CMeshSphereShellDialog(QWidget* parent, const char* name):
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


CMeshSphereShellDialog::~CMeshSphereShellDialog(void)
{
}


void CMeshSphereShellDialog::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Outer Radius", 'd', &m_fR);
	m_table[i++]=CNameTableItem("Shell Thickness", 'd', &m_fH);
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Beta1", 'd', &m_fB1);
	m_table[i++]=CNameTableItem("Beta2", 'd', &m_fB2);

	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Div.", 'i', &m_nArcDiv);
	m_table[i++]=CNameTableItem("Thickness Div.", 'i', &m_nHgtDiv);
}


void CMeshSphereShellDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_fR<=0) return;
	if (m_fH<=0) return;
	if (m_fH>=m_fR) return;
	if (m_nArcDiv<2) m_nArcDiv=2;
	if (m_nHgtDiv<1) m_nHgtDiv=1;
	int nzone1, nzone2;
	FREAL zoneangle1, zoneangle2;
	GET_ANGULAR_SECTIONS(m_fA1, m_fA2, 360, nzone1, zoneangle1);
	GET_ANGULAR_SECTIONS(m_fB1, m_fB2, 180, nzone2, zoneangle2);
	zoneangle2 = 90;
	if (nzone1==0 || nzone2==0) return;
	int nArcDiv=m_nArcDiv/nzone1; 
	if (nArcDiv<2) nArcDiv=2;

	//Create the first quater cyl object
	CHexObj *pobj;
	const FREAL a1=m_fA1;
	const FREAL a2=a1+zoneangle1;
	FREAL b1;
	if (m_fB1<90) b1=0;
	else b1=90;
	const FREAL b2=b1+zoneangle2;
	CHexObj *p1;
	if (b1==0)
		p1=meshQuarterSphereShell(m_fR, m_fH, a1, a2, b1, b2, nArcDiv, m_nHgtDiv);
	else{
		p1=meshQuarterSphereShell(m_fR, m_fH, a1, a2, 0, 90, nArcDiv, m_nHgtDiv);
		p1->mirrorXYPlane();
	}
	if (nzone2==2){
		CHexObj *p2 = new CHexObj(*p1);
		CHexObj *p3 = new CHexObj;
		assert(p2!=NULL && p3!=NULL);
		p2->mirrorXYPlane();
		MergeHexObj(*p1, *p2, *p3);
		delete p1; delete p2;
		p1 = p3;
	}

	//Make copies of the first part
	if (p1==NULL) return;
	pobj = p1;
	for (int i=1; i<nzone1; i++){
		CHexObj *pobj0=pobj;
		CHexObj *pdup = new CHexObj(*p1);
		pobj = new CHexObj;
		FAST_ROTATE_Z_AXIS(pdup->m_pVertex, pdup->m_nVertexCount, zoneangle1);
		MergeHexObj(*pobj0, *pdup, *pobj);
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