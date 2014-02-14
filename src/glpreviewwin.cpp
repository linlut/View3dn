
//FILE: GLPREVIEWWIN.CPP
#include "glpreviewwin.h"
#include "sphere16.h"
#include "gllight.h"


using namespace std;
using namespace qglviewer;


void CGLPreviewWin::_initWindow(void)
{
	m_pObj = NULL;
	m_DrawParms.m_bEnableLighting = true;
	m_DrawParms.m_cObjectColor = CGLDrawParms::previewNewColor();
	m_DrawParms.m_nDrawType=CGLDrawParms::DRAW_MESH_HIDDENLINE;
	m_DrawParms.m_cHLineColor = Vector3f(0,0,0);
	m_DrawParms.m_cBackgroundColor = Vector3f(0.5f);

	m_pNameTable = NULL;
	m_nTableLen = 0;
	m_nTableType = 0;

	m_inRenderingFunc = 0;
	m_timerStep = new QTimer(this);
	assert(m_timerStep!=NULL);
	connect(m_timerStep, SIGNAL(timeout()), this, SLOT(updateGL()) );
	m_timerStep->start(100);	//10HZ timeout
}


void CGLPreviewWin::_draw(void)
{
	//setup lights
	{
		Vec pos=camera()->position();
		Vector3f lightpos0(pos.x, pos.y, pos.z);
		CPointLight light0(lightpos0);
		light0.applyLight(0);
	}

	if (m_pObj==NULL){ 
		return;
	}

	glPushMatrix();
		m_DrawParms.BeginDrawing();
			m_pObj->glDraw(m_DrawParms);
		m_DrawParms.PostDrawing();
	glPopMatrix();

	//glEnable(GL_DEPTH_TEST);
	//_mydrawAxis();
}


void CGLPreviewWin::_drawNameTable(void)
{
	int i;
	unsigned char redclr[3]={255,0,0};
	unsigned char txtclr[3]={0,0,0};
	Camera *pcam = this->camera();
	
	//get the object boundingbox
    double r;
	if (m_pObj==NULL) r=0.01f;
    else{
	    AxisAlignedBox box;
	    m_pObj->GetBoundingBox(box);
	    Vector3d dist = box.maxp - box.minp;
	    r = fabs(dist.x+dist.y+dist.z) *0.006;
    }

	//draw the vertices as spheres
	GLUquadric * pquad = gluNewQuadric();
	assert(pquad!=NULL);
	gluQuadricDrawStyle(pquad, GLU_FILL);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glColor3ub(redclr[0], redclr[1], redclr[2]);
		for (i=0; i<m_nTableLen; i++){
			CNameTableItem *p = &m_pNameTable[i];
			if (p->m_Type=='V'){
				const double *wpos = (double*)p->m_pAddr;
				const Vector3d wrdpos(wpos[0], wpos[1], wpos[2]);
				CSphere16::getInstance().glDraw(wrdpos, r, false);
			}
		}
	gluDeleteQuadric(pquad);
	glDisable(GL_COLOR_MATERIAL);

	//draw the text
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	setTextIsEnabled(true);
	glColor3ub(txtclr[0], txtclr[1], txtclr[2]);

	startScreenCoordinatesSystem();
	for (i=0; i<m_nTableLen; i++){
		CNameTableItem *p = &m_pNameTable[i];
		if (p->m_Type=='V'){
			const double *wpos = (const double*)p->m_pAddr;
			const char *pname = p->m_sName;
			const Vec wrdpos(wpos[0], wpos[1], wpos[2]);
			const Vec scrpos = pcam->projectedCoordinatesOf(wrdpos);
			const int posx = scrpos.x;
			const int posy = scrpos.y;
			drawText(posx+4, posy+2, pname);
		}
	}
	stopScreenCoordinatesSystem();
	glEnable(GL_DEPTH_TEST);
}


CGLPreviewWin::CGLPreviewWin(QWidget *parent, const char *windowname, const QGLWidget* shareWidget):
	QGLViewer(parent, windowname, shareWidget), 
	m_DrawParms()
{
	_initWindow();
}


CGLPreviewWin::~CGLPreviewWin()
{
	SafeDelete(m_pObj);
	SafeDelete(m_timerStep);
}


//before the first opengl draw screen is called
void CGLPreviewWin::init(void)
{
	Vector3f bg = m_DrawParms.m_cBackgroundColor * 255.0f;
	setBackgroundColor(QColor(bg.x, bg.y, bg.z));
	setAxisIsDrawn();
	toggleGridIsDrawn();
	setManipulatedFrame(NULL);

	Camera *pcam = camera();
	pcam->setFlySpeed(0);
	pcam->frame()->setSpinningSensitivity(8);
}


void CGLPreviewWin::setObject(CPolyObj *pobj)
{
	//copy the object
	if (m_pObj && (m_pObj!=pobj)) delete m_pObj;
	m_pObj = pobj;

	//set the rendering related issues;
	Vec boxmin, boxmax, viewcenter;
	if (pobj==NULL){
		boxmin=Vec(-0.5f, -0.5f, -0.5f);
		boxmax=Vec(+0.5f, +0.5f, +0.5f);
	}
	else{
		AxisAlignedBox box;
		pobj->GetBoundingBox(box);
		double *t = &box.minp.x;
		boxmin=Vec(t[0], t[1], t[2]);
		t = &box.maxp.x;
		boxmax=Vec(t[0], t[1], t[2]);
	}
	viewcenter = (boxmin+boxmax)*0.5f;
	Camera *pcam = camera();
	pcam->setSceneBoundingBox(boxmin, boxmax);
	pcam->setSceneCenter(viewcenter);
	pcam->showEntireScene();
}


void CGLPreviewWin::keyPressEvent( QKeyEvent * e )
{
	const int key = e->key();
    bool redraw=false;
	switch(key){
	case Qt::Key_Escape:
	case Qt::Key_S:
		break;
	case Qt::Key_F:		//flat shading
	    m_DrawParms.m_nDrawType=CGLDrawParms::DRAW_MESH_SHADING;
        redraw = true;
		break;
	case Qt::Key_H:		//hidden line rendering
    	m_DrawParms.m_nDrawType=CGLDrawParms::DRAW_MESH_HIDDENLINE;
        redraw = true;
        break;
	case Qt::Key_L:		//line rendering
	    m_DrawParms.m_nDrawType=CGLDrawParms::DRAW_MESH_LINE;
        redraw = true;
        break;
    default:
        QGLViewer::keyPressEvent(e);
        break;
    }
    if (redraw) updateGL();  
}
