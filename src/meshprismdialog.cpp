//FILE: meshprismdialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshprismdialog.h"
#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"



//=======================================================================
static void 
cylSubzionToHexObj(MV3& vertex, const int nx, const int nsplith, FREAL h, CHexObj &obj)
{ 
	int _tmp, ny, nz, c, i, j;
	vertex.getMatrixDimension(_tmp, ny, nz);
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
				meshvert[c] = vertex(i, j, k);
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


static CHexObj* 
meshPrism(const Vector3d v[6], const FREAL& h, const int nsplitw, const int nsplith)
{
	MV3 vertex;
	M2 xyzo(3,6);

	//create the sphere elements
	Vector3d vv[6];
	for (int i=0; i<6; i++){
		vv[i].x = v[i].x; vv[i].y = v[i].y; vv[i].z = h;
	}
	prismMesher(vv, nsplitw, vertex);

	//separate the 3 zones
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nx3 = nx/3;
	const int nsize = nx3*ny*nz;
	assert(nx%3==0);
	MV3 cyl1=vertex(1);
	MV3 cyl2=vertex(1+nx3);
	MV3 cyl3=vertex(1+nx3+nx3);
	CHexObj tmpObj, *pQuarterCyl=new CHexObj;
	assert(pQuarterCyl!=NULL);
	{
		CHexObj *obj1 = new CHexObj;
		CHexObj *obj2 = new CHexObj;
		assert(obj1!=NULL && obj2!=NULL);
		cylSubzionToHexObj(cyl1, nx3, nsplith, h, *obj1);
		cylSubzionToHexObj(cyl2, nx3, nsplith, h, *obj2);
		MergeHexObj(*obj1, *obj2, tmpObj);
		delete obj1;
		delete obj2;
	}
	{
		CHexObj *obj3= new CHexObj;
		assert(obj3!=NULL);
		cylSubzionToHexObj(cyl3, nx3, nsplith, h, *obj3);
		MergeHexObj(tmpObj, *obj3, *pQuarterCyl);
		delete obj3;
	}

	return pQuarterCyl;	
}


//========================================================================

const double RR=0.5f;
const double RR2=0.5f*RR;
const double TT=RR*0.5*sqrt(3.00);
Vector3d CMeshPrismDialog::m_vVertex[6] = {
	Vector3d(RR, 0, 0), 
	Vector3d(RR2, TT, 0),
	Vector3d(-RR2, TT, 0),
	Vector3d(-RR, 0, 0),
	Vector3d(-RR2, -TT, 0),
	Vector3d(RR2, -TT, 0)};
double CMeshPrismDialog::m_fH = 1;
Vector3d CMeshPrismDialog::m_vTranslate = Vector3d(0,0,0);
int CMeshPrismDialog::m_nArcDiv=4;
int CMeshPrismDialog::m_nHgtDiv=4;


CMeshPrismDialog::CMeshPrismDialog(QWidget* parent, const char* name):
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


CMeshPrismDialog::~CMeshPrismDialog(void)
{
}


void CMeshPrismDialog::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Vertex 1", 'V', &m_vVertex[0]);
	m_table[i++]=CNameTableItem("Vertex 2", 'V', &m_vVertex[1]);
	m_table[i++]=CNameTableItem("Vertex 3", 'V', &m_vVertex[2]);
	m_table[i++]=CNameTableItem("Vertex 4", 'V', &m_vVertex[3]);
	m_table[i++]=CNameTableItem("Vertex 5", 'V', &m_vVertex[4]);
	m_table[i++]=CNameTableItem("Vertex 6", 'V', &m_vVertex[5]);

	m_table[i++]=CNameTableItem("Height", 'd', &m_fH);
	m_table[i++]=CNameTableItem("Center", 'V', &m_vTranslate);
	m_table[i++]=CNameTableItem("Arc Division", 'i', &m_nArcDiv);
	m_table[i++]=CNameTableItem("Height Division", 'i', &m_nHgtDiv);
}



void CMeshPrismDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check parameters
	if (m_fH==0) return;
	if (m_nArcDiv<1) m_nArcDiv=1;
	if (m_nHgtDiv<1) m_nHgtDiv=1;

	//Create the first quater cyl object
	CHexObj *pobj;
	CHexObj* p1=meshPrism(m_vVertex, m_fH, m_nArcDiv, m_nHgtDiv);
	if (p1==NULL) return;
	pobj = p1;

	//Dont forget to translate m_vCenter;
	if (!(m_vTranslate==Vector3d(0,0,0))){
		const double tx = m_vTranslate.x;
		const double ty = m_vTranslate.y;
		const double tz = m_vTranslate.z;		
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


