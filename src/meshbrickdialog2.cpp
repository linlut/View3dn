//FILE: meshbrickdialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>
#include "meshbrickdialog2.h"
#include "hexobj.h"
#include "glpreviewwin.h"
#include "fortranarray.h"

#define F5 (0.5)
#define F25 (0.25)

int CMeshSlantBrickDialog::m_nx = 3;
int CMeshSlantBrickDialog::m_ny = 4;
int CMeshSlantBrickDialog::m_nz = 5;
Vector3d CMeshSlantBrickDialog::m_vVertex[8]={
		Vector3d(-F25,-F25,-F5),
		Vector3d(F25, -F25,-F5),
		Vector3d(F25, F25, -F5),
		Vector3d(-F25,F25, -F5),
		Vector3d(-F5,-F5,+F5),
		Vector3d(F5, -F5,+F5),
		Vector3d(F5, F5, +F5),
		Vector3d(-F5,F5, +F5)
};


CMeshSlantBrickDialog::CMeshSlantBrickDialog(QWidget* parent, const char* name):
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


CMeshSlantBrickDialog::~CMeshSlantBrickDialog(void)
{
}


void CMeshSlantBrickDialog::prepareNameTable( void )
{
	int & i = m_nLen;
	i=0;
	m_table[i++]=CNameTableItem(" V1 ", 'V', &m_vVertex[0]);
	m_table[i++]=CNameTableItem(" V2 ", 'V', &m_vVertex[1]);
	m_table[i++]=CNameTableItem(" V3 ", 'V', &m_vVertex[2]);
	m_table[i++]=CNameTableItem(" V4 ", 'V', &m_vVertex[3]);
	m_table[i++]=CNameTableItem(" V5 ", 'V', &m_vVertex[4]);
	m_table[i++]=CNameTableItem(" V6 ", 'V', &m_vVertex[5]);
	m_table[i++]=CNameTableItem(" V7 ", 'V', &m_vVertex[6]);
	m_table[i++]=CNameTableItem(" V8 ", 'V', &m_vVertex[7]);

	m_table[i++]=CNameTableItem("NX", 'i', &m_nx);
	m_table[i++]=CNameTableItem("NY", 'i', &m_ny);
	m_table[i++]=CNameTableItem("NZ", 'i', &m_nz);
}

void CMeshSlantBrickDialog::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Create a new brick object
	const int nx = this->m_nx;
	const int ny = this->m_ny;
	const int nz = this->m_nz;
	Vector3d* pVertex;
	Vector4i* pQuad;
	Vector8i* pHex;
	int nVertexCount, nPolygonCount, nHexCount;
	//doMeshBrick(lowleft, upright, nx, ny, nz, pVertex, pQuad, pHex, nVertexCount, nPolygonCount, nHexCount);
	meshSlantBrickVertices(m_vVertex, nx+1, ny+1, nz+1, pVertex, nVertexCount);
	getBrickElements(nx, ny, nz, pHex, nHexCount);
	getBrickBounday(nx, ny, nz, pQuad, nPolygonCount);

	CHexObj *p = new CHexObj(pVertex, nVertexCount, pHex, nHexCount, pQuad, nPolygonCount);
	if (p==NULL) return;
	Vector3d lowleft(+1e30f); 
	Vector3d upright(-1e30f);
	for (int i=0; i<8; i++){
        const Vector3d vvv(m_vVertex[i].x, m_vVertex[i].y, m_vVertex[i].z);
		Minimize(lowleft, vvv);
		Maximize(upright, vvv);
	}
	p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
	char namebuff[256];
	sprintf(namebuff, "Brick%d", 0);
	p->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(p);
	m_pGLPreviewWin->updateGL();  	
}


