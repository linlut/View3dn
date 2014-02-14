//FILE: meshsphereshelldialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshcylindershelldialog.h"
#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


//=======================================================================
/*
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
	getBrickElements(nx-1, ny-1, nz-1, obj.m_pHex, obj.m_nHexCount);
	getBrickBounday(nx-1, ny-1, nz-1, obj.m_pQuad, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
}


static CHexObj* meshQuarterCylinderShell(
	const FREAL& r, const FREAL &h, const FREAL &th, const FREAL& a0, const FREAL& a1, 
	const int nsplitw, const int nsplitt, const int nsplith)
{
	int nx, ny, nz;
	MV3 vertex;
	cylinderShell(nsplitw, nsplitt, nsplith, r, h, th, a0, a1, vertex);
	vertex.getMatrixDimension(nx, ny, nz);
	CHexObj *pQuarterObj = new CHexObj;
	assert(pQuarterObj!=NULL);
	sphereSubzionToHexObj(vertex, nx, *pQuarterObj);
	return pQuarterObj;
}
*/


static void 
cylSubzoneToHexObj(MV3& vertex, const int nx, const int nsplith, FREAL h, CHexObj &obj)
{ 
	int _tmp, ny, nz, c, i, j;
	vertex.getMatrixDimension(_tmp, ny, nz);
	nz=1;
	assert(_tmp>=nx);
	const int nsize = nx*ny*nz;
	const int nv = nsize * (nsplith+1);
	Vector3d *meshvert = new Vector3d[nv];
	assert(meshvert!=NULL);

	//copy the first layer vertices
	c = nsplith*nsize;
	for (int k=1; k<=nz; k++){
		for (j=1; j<=ny; j++){
			for (i=1; i<=nx; i++, c++)
				meshvert[c] = vertex(i, j, 2);
		}
	}
	//duplicate the rest layers from the first layer
	for (i=0; i<nsplith; i++){
		Vector3d *psrc = &meshvert[nsplith*nsize];
		Vector3d *pdst = &meshvert[i*nsize];
		memcpy(pdst, psrc, nsize*sizeof(Vector3d));
	}
	for (c=j=0; j<nsplith; j++){
		const double dz = (h*(nsplith-j))/(double)nsplith;
		const double zval = meshvert[c].z-dz;
		for (i=0; i<nsize; i++, c++) meshvert[c].z = zval;
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


static CHexObj* meshQuarterCylinderShell(
	const FREAL& r, const FREAL &h, const FREAL &th, const FREAL& a0, const FREAL& a1, 
	const int nsplitw, const int nsplitt, const int nsplith)
{
	MV3 vertex;
	M1 alpha(2);

	//create the sphere elements
	alpha(1)=a0, alpha(2)=a1;
	cylinderShell(nsplitw, nsplitt, 1, r, h, th, a0, a1, vertex);

	//separate the 2 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx3 = nx/2;
	MV3 cyl1=vertex(1);
	MV3 cyl2=vertex(1+nx3);
	CHexObj _tmpObj1, _tmpObj2, *pQuarterCyl=new CHexObj;
	assert(pQuarterCyl!=NULL);
	cylSubzoneToHexObj(cyl1, nx3, nsplith, h, _tmpObj1); 
	cylSubzoneToHexObj(cyl2, nx3, nsplith, h, _tmpObj2);
	MergeHexObj(_tmpObj1, _tmpObj2, *pQuarterCyl);
	return pQuarterCyl;
}



//====================================================================
int CMeshCylinderShellDialog::m_nArcDiv = 8;
int CMeshCylinderShellDialog::m_nThkDiv = 2;
int CMeshCylinderShellDialog::m_nHgtDiv = 4;
double CMeshCylinderShellDialog::m_fR =  0.5000;
double CMeshCylinderShellDialog::m_fHeight = 1.000;
double CMeshCylinderShellDialog::m_fThickness = 0.1000;
double CMeshCylinderShellDialog::m_fA1 = 0;
double CMeshCylinderShellDialog::m_fA2 = 90;
Vector3d CMeshCylinderShellDialog::m_vCenter = Vector3d(0,0,0);


CMeshCylinderShellDialog::CMeshCylinderShellDialog(QWidget* parent, const char* name):
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


CMeshCylinderShellDialog::~CMeshCylinderShellDialog(void)
{
}


void CMeshCylinderShellDialog::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Outer Radius", 'd', &m_fR);
	m_table[i++]=CNameTableItem("Shell Thickness", 'd', &m_fThickness);
	m_table[i++]=CNameTableItem("Height", 'd', &m_fHeight);
	m_table[i++]=CNameTableItem("Alpha1", 'd', &m_fA1);
	m_table[i++]=CNameTableItem("Alpha2", 'd', &m_fA2);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Arc Div #", 'i', &m_nArcDiv);
	m_table[i++]=CNameTableItem("Thick Div #", 'i', &m_nThkDiv);
	m_table[i++]=CNameTableItem("Height Div #", 'i', &m_nHgtDiv);
}


void CMeshCylinderShellDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_fR<=0) return;
	if (m_fHeight<=0) return;
	if (m_fThickness<=0) return;
	if (m_nArcDiv<2) m_nArcDiv=2;
	if (m_nThkDiv<1) m_nThkDiv=1;
	if (m_nHgtDiv<1) m_nHgtDiv=1;
	int nzone;
	FREAL zoneangle;
	GET_ANGULAR_SECTIONS(m_fA1, m_fA2, 360, nzone, zoneangle);
	if (nzone==0) return;
	int nArcDiv = m_nArcDiv/nzone;
	if (nArcDiv<2) nArcDiv=2;

	//Create the first quater cyl object
	CHexObj *pobj;
	const FREAL a1=m_fA1;
	const FREAL a2=m_fA1+zoneangle;
	CHexObj *p1 = meshQuarterCylinderShell(m_fR, m_fHeight, m_fThickness, a1, a2, nArcDiv, m_nThkDiv, m_nHgtDiv);

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
	sprintf(namebuff, "CylinderShell%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  
}