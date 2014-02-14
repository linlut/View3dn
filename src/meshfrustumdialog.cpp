//FILE: meshspheredialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"
#include "meshfrustumdialog.h"



//=======================================================================
static void 
cylSubzionToHexObj(MV3& vertex1, MV3& vertex2, const int nx, const int nsplith, FREAL h, CHexObj &obj)
{ 
	int _tmp, ny, nz, i, j;
	vertex1.getMatrixDimension(_tmp, ny, nz);
	assert(nz==1);
	assert(_tmp>=nx);
	const int nsize = nx*ny*nz;
	const int nv = nsize * (nsplith+1);
	Vector3d *meshvert = new Vector3d[nv];
	assert(meshvert!=NULL);

	//copy the first layer vertices
	Vector3d *p1 = meshvert;
	Vector3d *p2 = meshvert+(nsplith*nsize);
	for (int k=1; k<=nz; k++){
		for (j=1; j<=ny; j++){
			for (i=1; i<=nx; i++, p1++, p2++){
				*p1 = vertex1(i, j, k);
				p1->z = 0;
				*p2 = vertex2(i, j, k);
				p2->z = h;
			}
		}
	}

	for (j=1; j<nsplith; j++){
		p1 = meshvert;
		p2 = meshvert+(nsplith*nsize);
		Vector3d *p = meshvert+(j*nsize);
		const double t = j/((double)nsplith);
		const double t1 = 1-t;
		for (i=0; i<nsize; i++, p++, p1++, p2++){
			*p = (*p1)*t1+(*p2)*t;
		}
	}
	obj.m_nVertexCount = nv;
	obj.m_pVertex = meshvert;
	Vector8i *&objm_pHex = (Vector8i *&)obj.m_pElement;
	getBrickElements(nx-1, ny-1, nsplith, objm_pHex, obj.m_nElementCount);
	Vector4i * &m_pQuad = (Vector4i * &)obj.m_pPolygon;
	getBrickBounday(nx-1, ny-1, nsplith, m_pQuad, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
}


static CHexObj* meshQuarterCylinder(
	const Vector3f& ct, const FREAL& r1, const FREAL& r2, const FREAL& a0, const FREAL& a1, const FREAL& h, 
	const int nsplitw, const int nsplith)
{
	MV3 vertex1, vertex2;
	M1 alpha(2);

	//create the sphere elements
	alpha(1)=a0, alpha(2)=a1;
	colmshzn(nsplitw+1, r1, alpha, h, vertex1);
	colmshzn(nsplitw+1, r2, alpha, h, vertex2);

	//separate the 3 zones
	int nx, ny, nz;
	vertex1.getMatrixDimension(nx, ny, nz);
	assert(nz==1);
	const int nx3 = nx/3;
	const int nsize = nx3*ny*nz;
	assert(nx%3==0);
	MV3 cyl1=vertex1(1);
	MV3 cyl2=vertex1(1+nx3);
	MV3 cyl3=vertex1(1+nx3+nx3);
	MV3 fyl1=vertex2(1);
	MV3 fyl2=vertex2(1+nx3);
	MV3 fyl3=vertex2(1+nx3+nx3);
	CHexObj tmpObj, *pQuarterCyl=new CHexObj;
	assert(pQuarterCyl!=NULL);

	{
		CHexObj *obj1 = new CHexObj;
		CHexObj *obj2 = new CHexObj;
		assert(obj1!=NULL && obj2!=NULL);
		cylSubzionToHexObj(cyl1, fyl1, nx3, nsplith, h, *obj1);
		cylSubzionToHexObj(cyl2, fyl2, nx3, nsplith, h, *obj2);
		MergeHexObj(*obj1, *obj2, tmpObj);
		delete obj1;
		delete obj2;
	}
	{
		CHexObj *obj3= new CHexObj;
		assert(obj3!=NULL);
		cylSubzionToHexObj(cyl3, fyl3, nx3, nsplith, h, *obj3);
		MergeHexObj(tmpObj, *obj3, *pQuarterCyl);
		delete obj3;
	}

	return pQuarterCyl;
}



//========================================================================

int CMeshFrustumDialog::m_nDivArc=4;
int CMeshFrustumDialog::m_nDivHgt=10;
double CMeshFrustumDialog::m_fR1 =  1.00;
double CMeshFrustumDialog::m_fR2 =  0.50;
double CMeshFrustumDialog::m_fA1 = 0;
double CMeshFrustumDialog::m_fA2 = 90;
double CMeshFrustumDialog::m_fH = 1;
Vector3d CMeshFrustumDialog::m_vCenter = Vector3d(0,0,0);


CMeshFrustumDialog::CMeshFrustumDialog(QWidget* parent, const char* name):
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


CMeshFrustumDialog::~CMeshFrustumDialog(void)
{
}


void CMeshFrustumDialog::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Radius Bottom", 'd', &m_fR1);
	m_table[i++]=CNameTableItem("Radius Top", 'd', &m_fR2);
	m_table[i++]=CNameTableItem("Height", 'd', &m_fH);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);

	m_table[i++]=CNameTableItem("Arc Division", 'i', &m_nDivArc);
	m_table[i++]=CNameTableItem("Height Division", 'i', &m_nDivHgt);
}



void CMeshFrustumDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check parameters
	if (m_fR1<=0 || m_fR2<=0) return;
	if (m_nDivArc<1) m_nDivArc=1;
	if (m_nDivHgt<1) m_nDivHgt=1;
	int nzone;
	FREAL zoneangle;
	GET_ANGULAR_SECTIONS(m_fA1, m_fA2, 360, nzone, zoneangle);
	if (nzone==0) return;
	int nDivArc = m_nDivArc/nzone;
	if (nDivArc<1) nDivArc=1;

	//Create the first quater cyl object
	CHexObj *pobj;
	CHexObj* p1=meshQuarterCylinder(Vector3f(0), m_fR1, m_fR2, m_fA1, m_fA1+zoneangle, m_fH, nDivArc, m_nDivHgt);
	if (p1==NULL) return;
	pobj = p1;
	for (int i=1; i<nzone; i++){
		CHexObj *pobj0=pobj;
		CHexObj *pdup = new CHexObj(*p1);
		pobj = new CHexObj;
		FAST_ROTATE_Z_AXIS(pdup->m_pVertex, pdup->m_nVertexCount, zoneangle);
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
	sprintf(namebuff, "Cylinder%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  	
}


