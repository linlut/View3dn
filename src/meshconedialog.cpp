//FILE: meshspheredialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"
#include "meshconedialog.h"


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


static CHexObj* meshQuarterCone(
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
	MV3 obj1=vertex(1);
	MV3 obj2=vertex(1+nx4);
	MV3 obj3=vertex(1+nx4*2);
	MV3 obj4=vertex(1+nx4*3);

	CHexObj tmpObj1, tmpObj2;
	{
		CHexObj *pobj1 = new CHexObj;
		CHexObj *pobj2 = new CHexObj;
		assert(pobj1!=NULL && pobj2!=NULL);
		sphereSubzionToHexObj(obj1, nx4, *pobj1);
		sphereSubzionToHexObj(obj2, nx4, *pobj2);
		MergeHexObj(*pobj1, *pobj2, tmpObj1);
		delete pobj1;
		delete pobj2;
	}
	{
		CHexObj *pobj1 = new CHexObj;
		CHexObj *pobj2 = new CHexObj;
		assert(pobj1!=NULL && pobj2!=NULL);
		sphereSubzionToHexObj(obj3, nx4, *pobj1);
		sphereSubzionToHexObj(obj4, nx4, *pobj2);
		MergeHexObj(*pobj1, *pobj2, tmpObj2);
		delete pobj1;
		delete pobj2;
	}
	
	CHexObj *pQuarterObj = new CHexObj;
	assert(pQuarterObj!=NULL);
	MergeHexObj(tmpObj1, tmpObj2, *pQuarterObj);

	return pQuarterObj;
}


//====================================================================
int CMeshConeDialog::m_nArcDiv = 4;
int CMeshConeDialog::m_nHgtDiv = 4;
double CMeshConeDialog::m_fR =  0.8;
double CMeshConeDialog::m_fH =  1.0;
double CMeshConeDialog::m_fA1 = 0;
double CMeshConeDialog::m_fA2 = 90;
Vector3d CMeshConeDialog::m_vCenter = Vector3d(0,0,0);


CMeshConeDialog::CMeshConeDialog(QWidget* parent, const char* name):
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


CMeshConeDialog::~CMeshConeDialog(void)
{
}


void CMeshConeDialog::prepareNameTable( void )
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


void CMeshConeDialog::onApply(void)
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
	CHexObj *pobj;
	const FREAL a1=m_fA1;
	const FREAL a2=m_fA1+zoneangle;
	CHexObj *p1 = meshQuarterCone(m_fR, m_fH, a1, a2, nArcDiv, nArcDiv);
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
	sprintf(namebuff, "Cone%d", 0);
	pobj->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(pobj);
	m_pGLPreviewWin->updateGL();  	
}
