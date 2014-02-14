//FILE: hlineview.cpp

#include <qfiledialog.h>

#include "hlineview.h"
#include "hline_removal.h"
#include "tmeshobj.h"
#include "qmeshobj.h"



inline Vector3f tofloatvec(double v[3])
{
	Vector3f r(v[0], v[1], v[2]);
	return r;
}

inline Vector3f GetTriangleNormal(double p1[3], double p2[3], double p3[3])
{
	Vector3f v1=tofloatvec(p1);
	Vector3f v2=tofloatvec(p2);
	Vector3f v3=tofloatvec(p3);
	Vector3f n=compute_triangle_normal(v1, v2, v3);
	return n;
}


void CHlineViewer::convertTriangleObj2Arface(CTriangleObj *ptri, ARFACE** facets)
{
    /*
	int i, nface;
	assert(ptri!=NULL);
	assert(facets!=NULL);
	nface = ptri->m_nPolygonCount;
	Vector3d *pvert = ptri->m_pVertex;
    Vector3d p1, p2, p3;
	double v[3];
	Vector3i*& pTriangle2 = (Vector3i*&)ptri->m_pPolygon;

	for (i=0; i<nface; i++){
		//allocate each triangle a space;
		facets[i]=(ARFACE*)m_pHLR->Malloc(sizeof(ARFACE));
		ARFACE *p = facets[i];
		assert(p!=NULL);
		p->flag = 0;
		p->np = 3;
		p->nh = 0;
		p->icolor = 0;		//black?
		p->hole_ptr=NULL;

		//copy vertex buffer;
		p->vertex = (POINT3D*)m_pHLR->Malloc(sizeof(POINT3D)* p->np);
		assert(p->vertex!=NULL);
		Vector3i& vlist= pTriangle2[i];
		p1 = ptri->m_pVertex[vlist.x];
		p2 = ptri->m_pVertex[vlist.y];
		p3 = ptri->m_pVertex[vlist.z];
		v[0]=p1.x, v[1]=p1.y, v[2]=p1.z;
		m_pHLR->m_pFuncWorld2Camera(v, p->vertex[0]);
		v[0]=p2.x, v[1]=p2.y, v[2]=p2.z;
		m_pHLR->m_pFuncWorld2Camera(v, p->vertex[1]);
		v[0]=p3.x, v[1]=p3.y, v[2]=p3.z;
		m_pHLR->m_pFuncWorld2Camera(v, p->vertex[2]);

		//copy plane equation;
		Vector3f n=GetTriangleNormal(p->vertex[0],p->vertex[1],p->vertex[2]);
		p->norm[0] = n.x;
		p->norm[1] = n.y;
		p->norm[2] = n.z;
		p->norm[3] = -(n.x*p->vertex[0][0]+n.y*p->vertex[0][1]+n.z*p->vertex[0][2]);
		p->ppf=NULL;		//Pointer to projected facet struct
		p->int_line=NULL;	//Pointer to intersection lines;
	}
    */
}


void CHlineViewer::convertQuadObj2Arface(CQuadObj *ptri, ARFACE** facets)
{
	int i, nface;
	assert(ptri!=NULL);
	assert(facets!=NULL);
	nface = ptri->m_nPolygonCount;
	Vector3d *pvert = ptri->m_pVertex;
	double v[3];

	Vector4i * &pQuadT = (Vector4i * &)ptri->m_pPolygon;
	for (i=0; i<nface; i++){
		//allocate each triangle a space;
		facets[i]=(ARFACE*)m_pHLR->Malloc(sizeof(ARFACE));
		ARFACE *p = facets[i];
		assert(p!=NULL);
		p->flag = 0;
		p->np = 4;
		p->nh = 0;
		p->icolor = 0;		//black?
		p->hole_ptr=NULL;

		//copy vertex buffer;
		p->vertex = (POINT3D*)m_pHLR->Malloc(sizeof(POINT3D)* p->np);
		assert(p->vertex!=NULL);
		Vector4i& vlist= pQuadT[i];
		Vector3d &p1 = ptri->m_pVertex[vlist.x];
		Vector3d &p2 = ptri->m_pVertex[vlist.y];
		Vector3d &p3 = ptri->m_pVertex[vlist.z];
		Vector3d &p4 = ptri->m_pVertex[vlist.w];
		v[0]=p1.x, v[1]=p1.y, v[2]=p1.z;
		m_pHLR->m_pFuncWorld2Camera(v, p->vertex[0]);
		v[0]=p2.x, v[1]=p2.y, v[2]=p2.z;
		m_pHLR->m_pFuncWorld2Camera(v, p->vertex[1]);
		v[0]=p3.x, v[1]=p3.y, v[2]=p3.z;
		m_pHLR->m_pFuncWorld2Camera(v, p->vertex[2]);
		v[0]=p4.x, v[1]=p4.y, v[2]=p4.z;
		m_pHLR->m_pFuncWorld2Camera(v, p->vertex[3]);

		//copy plane equation;
		Vector3f n=GetTriangleNormal(p->vertex[0],p->vertex[1],p->vertex[2]);
		p->norm[0] = n.x;
		p->norm[1] = n.y;
		p->norm[2] = n.z;
		p->norm[3] = -(n.x*p->vertex[0][0]+n.y*p->vertex[0][1]+n.z*p->vertex[0][2]);
		p->ppf=NULL;		//Pointer to projected facet struct
		p->int_line=NULL;	//Pointer to intersection lines;
	}
}



CHiddenLineRemoval* CHlineViewer::initHiddenlineClass(CGLDrawParms *pDrawParms, CSceneGraph *pSceneGraph)
{/*
	int i, npoly;
	assert(pSceneGraph!=NULL);
	int nnode = pSceneGraph->size();
	if (nnode<=0) return NULL;
	CHiddenLineRemoval *hlr = new CHiddenLineRemoval;
	assert(hlr!=NULL);

	//save var m_pHLR;
	CHiddenLineRemoval * ttt = m_pHLR;
	m_pHLR = hlr;
	for (i=npoly=0; i<nnode; i++){
		CSceneNode *p = pSceneGraph->GetSceneNode(i);
		npoly+=p->m_pObject->PolygonCount();
	}
	hlr->facets = new ARFACE*[npoly];
	assert(hlr->facets!=NULL);
	for (i=npoly=0; i<nnode; i++){
		CSceneNode *p = pSceneGraph->GetSceneNode(i);
		CObject3D *pobj = (CObject3D*)p->m_pObject;
		int nply0=pobj->PolygonCount();
		const char *des = pobj->Description();
		if (strcmp(des, "tri")==0){ //triangle object;
			convertTriangleObj2Arface((CTriangleObj *)pobj, hlr->facets+npoly);
			npoly+=nply0;
		}
		else if (strcmp(des, "qad")==0){ //quad object;
			convertQuadObj2Arface((CQuadObj *)pobj, hlr->facets+npoly);
			npoly+=nply0;
		}
	}
	
	//return;
	hlr->f_start = 0;
	hlr->f_end = npoly;
	hlr->facets_num = npoly;
	hlr->m_nOutputLineCount = 0;
	hlr->m_nOutputLineLimit = npoly*2+1024;
	hlr->m_OutputLineBuffer= new CElmLine2D[hlr->m_nOutputLineLimit];
	assert(hlr->m_OutputLineBuffer!=NULL);

	m_pHLR = ttt;
	return hlr;
    */

    return NULL;
}



extern void WorldPoint2Screen(double wp[3], double sp[3]);
extern void ScreenPoint2World(double sp[3], double wp[3]);
extern void WorldPoint2Camera(double wp[3], double cp[3]);

CHlineViewer::CHlineViewer(CGLDrawParms *pDrawParms, CSceneGraph *pSceneGraph, double mat[16], const int w, const int h, QWidget *parent,const char *name):
	QGLViewer(parent, name)	
{
    /*
	m_strFileName = name;
	m_pDrawParms = pDrawParms;
	m_pSceneGraph = pSceneGraph;

	//set transform functions and matrix;
	setCamera2ScreenMatrix(mat, w, h);
	m_pHLR = initHiddenlineClass(pDrawParms, pSceneGraph);

	m_pHLR->PreProcessing(w, h);
	//m_pHLR->HiddenLineRemoval();
	m_pHLR->FastHiddenLineRemoval();

	//set window size;
	resize(w, h);
	setCaption("Hiddenline view for: " + m_strFileName);

	updateGL();
    */
}


CHlineViewer::~CHlineViewer(void)
{
	m_pDrawParms = NULL;
	m_pSceneGraph = NULL;
	if (m_pHLR) delete m_pHLR;
}


void CHlineViewer::draw(void)
{
	//use the input data;
	float red   = m_pDrawParms->m_cBackgroundColor.x;
	float green = m_pDrawParms->m_cBackgroundColor.y;
	float blue  = m_pDrawParms->m_cBackgroundColor.z;
	setBackgroundColor(QColor(255*red, green*255, blue*255));

	if (m_pHLR==NULL)
		return;

	
	startScreenCoordinatesSystem();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3f(0,0,0);
	glBegin(GL_LINES);
	for (int i=0; i<m_pHLR->m_nOutputLineCount; i++){
		CElmLine2D* p = &m_pHLR->m_OutputLineBuffer[i];
		glVertex2f(p->x1, p->y1);
		glVertex2f(p->x2, p->y2);
	}
	glEnd();
	stopScreenCoordinatesSystem();
}


void CHlineViewer::setCamera2ScreenMatrix(double m[16], const int w, const int h)
{
	CHiddenLineRemoval::m_pFuncWorld2Screen = WorldPoint2Screen;
	CHiddenLineRemoval::m_pFuncScreen2World = ScreenPoint2World;
	CHiddenLineRemoval::m_pFuncWorld2Camera = WorldPoint2Camera;
/*
	double b1[4][4], b2[4][4];
	double *p = &b1[0][0]; 
	for (int i=0; i<16; i++)
		p[i] = m[i];

	TransposMatrix(b1, b2);
	UnitMatrix(b1);

	b1[0][0]=1*0.75; 
	b1[3][0]=0.5*w;
	b1[1][1]=-b1[0][0]; 
	b1[3][1]=0.5*h;
	b1[2][2]=2;
	b1[3][2]=-0.5;

	mmMult(b2, b1, m_pHLR->m_camera2sreenMatrix);
*/
	double *ptr = &m_pHLR->m_camera2sreenMatrix[0][0];
	for (int i=0; i<16; i++) ptr[i]=m[i];
	invertmat(m_pHLR->m_camera2sreenMatrix,m_pHLR->m_screen2cameraMatrix);
}



static void glwRedrawFunc(QGLViewer* w,void * user_data)
{
	glDisable(GL_DEPTH_TEST);
	CHlineViewer *hw = (CHlineViewer*)w;
	hw->draw();
}

void CHlineViewer::keyPressEvent( QKeyEvent * e )
{
	static QString saveDirPath;
	int key = e->key();
	if (key == Qt::Key_W){		//handle W key
		QString fname = QFileDialog::getSaveFileName(
						saveDirPath,
						"Windows meta file (*.wmf)",
						this,
						"Save WMF file dialog",
						"Choose an WMF file to save");
		if (!fname.isEmpty())
			m_pHLR->SaveWMFFile(fname.ascii());
	}
	else if (key == Qt::Key_S){	//handle S key
		extern bool ExportEPSFile(const char *fname, QGLViewer* win, void *drawfunc);
		QString selectedfilter = ".eps";
		QString fname = QFileDialog::getSaveFileName(
						saveDirPath,
						"EPS file (*.eps)",
						this,
						"Save EPS file dialog",
						"Choose an EPS file to save"
						);
		if (!fname.isEmpty()) 
			ExportEPSFile(fname.ascii(), this, (void*)glwRedrawFunc);
	}
	else
		QGLViewer::keyPressEvent(e);
}
