//FILE: meshbrickdialog.cpp
#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>
#include "mesh2drect_dlg.h"
#include "hexobj.h"
#include "glpreviewwin.h"


int CMesh2DRectDlg::m_nx = 4;
int CMesh2DRectDlg::m_ny = 4;
Vector3d CMesh2DRectDlg::m_vLowleft(-0.5, -0.5, 0);
Vector3d CMesh2DRectDlg::m_vUpright(0.5, 0.5, 0);
Vector3d CMesh2DRectDlg::m_vCenter(0);


CMesh2DRectDlg::CMesh2DRectDlg(QWidget* parent, const char* name):
	CMeshPrimitiveDialog(parent, name, false, Qt::WStyle_Tool)
{  
	setCaption(name);
	QValueList<int> slist;
	slist.append(80);
	slist.append(200);
//	m_pSplitter->setSizes(slist);

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


CMesh2DRectDlg::~CMesh2DRectDlg(void)
{
}


void CMesh2DRectDlg::prepareNameTable( void )
{
	int & i = m_nLen;
	i=0;
	m_table[i++]=CNameTableItem("x1", 'd', &m_vLowleft.x);
	m_table[i++]=CNameTableItem("y1", 'd', &m_vLowleft.y);
	m_table[i++]=CNameTableItem("x2", 'd', &m_vUpright.x);
	m_table[i++]=CNameTableItem("y2", 'd', &m_vUpright.y);
	m_table[i++]=CNameTableItem("Offset", 'V', &m_vCenter);
	m_table[i++]=CNameTableItem("NX", 'i', &m_nx);
	m_table[i++]=CNameTableItem("NY", 'i', &m_ny);
}


static inline int _decideDivDimension(const Vector3d& lowleft, const Vector3d& upright, int &nx, int &ny, int &nz)
{
    //find which plane is projected to
    int nn[3]={nx, ny, nz};
    Vector3d dif = upright-lowleft;
    dif.x = fabs(dif.x); 
    dif.y = fabs(dif.y); 
    dif.z = fabs(dif.z);
    double minxyz = _MIN3_(dif.x, dif.y, dif.z);
    int pos=-1, idx;
    for (int i=0; i<3; i++){
        const double *ptr = &dif.x;
        if (minxyz==ptr[i]){
            pos=i;
            break;
        }
    }   
   
    //find the axis, then set to zero
    nn[pos]=0;
    idx=(pos+1)%3;
    if (nn[idx]<1) nn[idx]=1;
    idx=(pos+2)%3;
    if (nn[idx]<1) nn[idx]=1;
    nx=nn[0]; ny=nn[1]; nz=nn[2];
    return pos;
}


CQuadObj *createRectSurf(const Vector3d& lowleft, const Vector3d& upright, const int nx, const int ny, const int nz)
{
	Vector3d* pVertex;
	Vector4i* pQuad;
	int nVertexCount, nPolygonCount;
	doMesh2dRectangle(lowleft, upright, nx, ny, nz, pVertex, pQuad, nVertexCount, nPolygonCount);
	CQuadObj *p = new CQuadObj(pVertex, nVertexCount, pQuad, nPolygonCount);
	return p;
}


void CMesh2DRectDlg::onApply(void)
{
	//Delete old object
    m_pGLPreviewWin->releaseObject();

	//Create a new brick object
    int nz=0;
	Vector3d lowleft = this->m_vLowleft;
	Vector3d upright = this->m_vUpright;
    upright.z = lowleft.z = 0;
    const int axis=_decideDivDimension(lowleft, upright, m_nx, m_ny, nz);
    if (axis!=2) return;
    if (nz!=0) return;
	CQuadObj *p = createRectSurf(lowleft, upright, m_nx, m_ny, nz);   
    if (p==NULL) return;

	//Dont forget to translate m_vCenter;
	if (!(m_vCenter==Vector3d(0,0,0))){
		const double tx = m_vCenter.x;
		const double ty = m_vCenter.y;
		const double tz = m_vCenter.z;		
		p->applyTranslation(tx, ty, tz);
        lowleft+=m_vCenter;
        upright+=m_vCenter;
	}

	p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
	char namebuff[256];
	sprintf(namebuff, "Brick%d", 0);
	p->SetObjectName(namebuff);

	//Set the new object for the preview window
	m_pGLPreviewWin->setObject(p);
	m_pGLPreviewWin->updateGL();  	
}


