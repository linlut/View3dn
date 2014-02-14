//=============================================
//
//FILE: meshcbrickdialog.cpp
//
//=============================================

#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include "meshcbrickdialog.h"
#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"


//=======================================================================
static void 
sphereSubzionToHexObj(MV3& vertex, CHexObj &obj)
{ 
	int nx, ny, nz;
	vertex.getMatrixDimension(nx, ny, nz);
	const int nsize = nx*ny*nz;
	Vector3d *meshvert = new Vector3d[nsize];
	assert(meshvert!=NULL);

	//copy the first layer vertices
	int c = 0;
	for (int k=1; k<=nz; k++){
		for (int j=1; j<=ny; j++){
			for (int i=1; i<=nx; i++, c++)
				meshvert[c] = vertex(i, j, k);
		}
	}
	obj.m_nVertexCount = nsize;
	obj.m_pVertex = meshvert;
	Vector4i *&m_pQuad = (Vector4i * &)obj.m_pPolygon;
	Vector8i *&pHex  = (Vector8i *&)obj.m_pElement;
	getBrickElements(nx-1, ny-1, nz-1, pHex, obj.m_nElementCount);
	getBrickBounday(nx-1, ny-1, nz-1, m_pQuad, obj.m_nPolygonCount);

	//set bounding box
	AxisAlignedBox box;
	obj.ComputeBoundingBox(box.minp, box.maxp);
	obj.SetBoundingBox(box);
}


static CHexObj* 
meshQuarterCylinderShell(const Vector3d v[20], const int nsx, const int nsy, const int nsz)
{
	int nx, ny, nz;
	MV3 vertex;
    msh20zn(v, nsx, nsy, nsz, vertex);

	vertex.getMatrixDimension(nx, ny, nz);
	CHexObj *pQuarterObj = new CHexObj;
	assert(pQuarterObj!=NULL);
	sphereSubzionToHexObj(vertex, *pQuarterObj);
	return pQuarterObj;
}


//====================================================================
int CMeshCbrickDialog::m_nArcDiv = 3;
int CMeshCbrickDialog::m_nThkDiv = 4;
int CMeshCbrickDialog::m_nHgtDiv = 5;
Vector3d CMeshCbrickDialog::m_vTrans = Vector3d(0,0,0);
Vector3d CMeshCbrickDialog::m_vVertex[20]={
	Vector3d(1,1,-1), 
	Vector3d(-1,1,-1), 
	Vector3d(-1,-1,-1), 
	Vector3d(1,-1,-1),
    
	Vector3d(1,1,+1), 
	Vector3d(-1,1,+1), 
	Vector3d(-1,-1,+1), 
	Vector3d(1,-1,+1), 
        
	Vector3d(0,1,-1), 
	Vector3d(-1,0,-1), 
	Vector3d(0,-1,-1), 
	Vector3d(1,0,-1), 

	Vector3d(1,1,0), 
	Vector3d(-1,1,0), 
	Vector3d(-1,-1,0), 
	Vector3d(1,-1,0), 

	Vector3d(0,1,+1), 
	Vector3d(-1,0,+1), 
	Vector3d(0,-1,+1), 
	Vector3d(1,0,+1)
};


CMeshCbrickDialog::CMeshCbrickDialog(QWidget* parent, const char* name):
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


CMeshCbrickDialog::~CMeshCbrickDialog(void)
{
}


void CMeshCbrickDialog::prepareNameTable( void )
{
	int & i = m_nLen;

	i=0;
	m_table[i++]=CNameTableItem("Vertex 1", 'V', &m_vVertex[0]);
	m_table[i++]=CNameTableItem("Vertex 2", 'V', &m_vVertex[1]);
	m_table[i++]=CNameTableItem("Vertex 3", 'V', &m_vVertex[2]);
	m_table[i++]=CNameTableItem("Vertex 4", 'V', &m_vVertex[3]);
	m_table[i++]=CNameTableItem("Vertex 5", 'V', &m_vVertex[4]);
	m_table[i++]=CNameTableItem("Vertex 6", 'V', &m_vVertex[5]);
	m_table[i++]=CNameTableItem("Vertex 7", 'V', &m_vVertex[6]);
	m_table[i++]=CNameTableItem("Vertex 8", 'V', &m_vVertex[7]);

	m_table[i++]=CNameTableItem("Midpoint 1-2", 'V', &m_vVertex[8]);
	m_table[i++]=CNameTableItem("Midpoint 2-3", 'V', &m_vVertex[9]);
	m_table[i++]=CNameTableItem("Midpoint 3-4", 'V', &m_vVertex[10]);
	m_table[i++]=CNameTableItem("Midpoint 4-1", 'V', &m_vVertex[11]);
	m_table[i++]=CNameTableItem("Midpoint 1-5", 'V', &m_vVertex[12]);
	m_table[i++]=CNameTableItem("Midpoint 2-6", 'V', &m_vVertex[13]);
	m_table[i++]=CNameTableItem("Midpoint 3-7", 'V', &m_vVertex[14]);
	m_table[i++]=CNameTableItem("Midpoint 4-8", 'V', &m_vVertex[15]);
	m_table[i++]=CNameTableItem("Midpoint 5-6", 'V', &m_vVertex[16]);
	m_table[i++]=CNameTableItem("Midpoint 6-7", 'V', &m_vVertex[17]);
	m_table[i++]=CNameTableItem("Midpoint 7-8", 'V', &m_vVertex[18]);
	m_table[i++]=CNameTableItem("Midpoint 8-5", 'V', &m_vVertex[19]);
	
	m_table[i++]=CNameTableItem("Translation", 'V', &m_vTrans);
	m_table[i++]=CNameTableItem("Arc Div #", 'i', &m_nArcDiv);
	m_table[i++]=CNameTableItem("Thick Div #", 'i', &m_nThkDiv);
	m_table[i++]=CNameTableItem("Height Div #", 'i', &m_nHgtDiv);
}


void CMeshCbrickDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Check input parameters
	if (m_nArcDiv<1) m_nArcDiv=1;
	if (m_nThkDiv<1) m_nThkDiv=1;
	if (m_nHgtDiv<1) m_nHgtDiv=1;

	//Create the first quater cyl object
	CHexObj *pobj;
	CHexObj *p1=meshQuarterCylinderShell(m_vVertex, m_nArcDiv, m_nThkDiv, m_nHgtDiv);

	if (p1==NULL) return;
	pobj = p1;

	//Dont forget to translate m_vCenter;
	if (!(m_vTrans==Vector3d(0,0,0))){
		const double tx = m_vTrans.x;
		const double ty = m_vTrans.y;
		const double tz = m_vTrans.z;		
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