//FILE: meshbrickdialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>
#include "meshbrickdialog.h"
#include "hexobj.h"
#include "glpreviewwin.h"


int CMeshBrickDialog::m_nx = 1;
int CMeshBrickDialog::m_ny = 1;
int CMeshBrickDialog::m_nz = 1;

Vector3d CMeshBrickDialog::m_vCenter(0);
Vector3d CMeshBrickDialog::m_vLowleft(-0.5);
Vector3d CMeshBrickDialog::m_vUpright(+0.5);


CMeshBrickDialog::CMeshBrickDialog(QWidget* parent, const char* name):
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


CMeshBrickDialog::~CMeshBrickDialog(void)
{
}


void CMeshBrickDialog::prepareNameTable( void )
{
	int & i = m_nLen;
	i=0;
	m_table[i++]=CNameTableItem("Center", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("Lowleft", 'V', &m_vLowleft);
	m_table[i++]=CNameTableItem("Upright", 'V', &m_vUpright);
	m_table[i++]=CNameTableItem("NX", 'i', &m_nx);
	m_table[i++]=CNameTableItem("NY", 'i', &m_ny);
	m_table[i++]=CNameTableItem("NZ", 'i', &m_nz);
}

void CMeshBrickDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Create a new brick object
	const Vector3d lowleft = this->m_vLowleft;
	const Vector3d upright = this->m_vUpright;
	const Vector3d center = this->m_vCenter;
	const int nx = this->m_nx;
	const int ny = this->m_ny;
	const int nz = this->m_nz;
	Vector3d* pVertex;
	Vector4i* pQuad;
	Vector8i* pHex;
	int nVertexCount, nPolygonCount, nHexCount;
	doMeshBrick(lowleft, upright, nx, ny, nz, pVertex, pQuad, pHex, nVertexCount, nPolygonCount, nHexCount);
	CHexObj *p = new CHexObj(pVertex, nVertexCount, pHex, nHexCount, pQuad, nPolygonCount);
	if (p==NULL) return;

	p->applyTranslation(center.x, center.y, center.z);
	p->SetBoundingBox(AxisAlignedBox(lowleft+center, upright+center));
	char namebuff[256];
	sprintf(namebuff, "Brick%d", 0);
	p->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(p);
	m_pGLPreviewWin->updateGL();  	
}


