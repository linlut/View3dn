//view3d.cpp
#ifdef WIN32
#include "windows.h"
#endif
#include <GL/glew.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qfont.h>
#include <qthread.h>
#include <zntoolbox.h>
#include "cia3dobj.h"
#include "scenegraph.h"
#include "scenegraphstate.h"
#include "view3d.h"
#include "gllight.h"
#include <GL/glut.h>
#include "ztime.h"


static void initGLEW(void)
{
	int err = glewInit();
	if (GLEW_OK != err){
		cerr << "Error: glewInit failed: " << (char*)glewGetErrorString(err) << endl;
		exit(1);
	}
}   

static unsigned int rendering_counter = 0;

void Viewer::init()
{
    //init GLEW and others
    initGLEW();
    extern int meshrender_initGL();
    meshrender_initGL();

	//restoreFromFile();
	int r = _pDrawParms->m_cBackgroundColor.x * 255;
	int g = _pDrawParms->m_cBackgroundColor.y * 255;
	int b = _pDrawParms->m_cBackgroundColor.z * 255;
	setBackgroundColor(QColor(r, g, b));

	//view angle is too large
	const double fov = PI/6.0;
	Camera *pcam = this->camera();
	pcam->setFieldOfView(fov);
	pcam->frame()->setSpinningSensitivity(3);

	//setup lights
	{
		Vector3f lightpos0(1000, 1000, 1000);
		CPointLight light0(lightpos0);
		//light0.applyLight(0);
	}

	{
		Vector3f lightpos1(-1000, -1000, -1000);
		CPointLight light1(lightpos1, 0.20f);
		light1.applyLight(1);
	}

	{
		Vector3f lightpos2(1000, 1000, 0);
		CPointLight light2(lightpos2, 0.20f);
		light2.applyLight(2);
	}
}


Viewer::Viewer(const QGLFormat &format, QWidget *parent,const char *name):
	QGLViewer(format, parent, name)
    ,m_animMutex()
    ,m_timer()
    ,m_useVbo(false)
    ,m_useDispList(false)
{
    setTextIsEnabled(false);
    setFPSIsDisplayed(false);

	_pDrawParms = NULL;	
	m_pBGImage = NULL;
	m_bFastDraw = false;
	m_bRepeatPlay = false;
	m_nTimeDelay = 0;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onDraw()));
    m_timer.start(0);
}


void Viewer::incEyeDistance(const int sign)
{
	const float INC_STEP=0.001f;  //1 mm in distance
	Camera *p = camera(); 
	float iodist = p->IODistance();
	iodist += INC_STEP*sign;
	cout<<"IODistance is "<<iodist<<endl;

	p->setIODistance(iodist);
	updateGL();
}


void Viewer::keyPressEvent( QKeyEvent * e )
{
	const int key = e->key();

	switch(key){  // see http://doc.trolltech.com/3.3/qt.html#Key-enum for values
		case Qt::Key_Escape:
			m_SceneGraph.setPickingType(CSceneGraph::PICK_NULL_TYPE);
			break;
		case Qt::Key_Tab:
			/*
			nsize = m_SceneGraph.SceneNodeCount();
			pActiveNode = m_SceneGraph.GetActiveSceneNode();
			if (pActiveNode==NULL || nsize==0)
				return;
			for (i=0; i<nsize; i++){
				CSceneNode * p = m_SceneGraph.GetSceneNode(i);
				if (pActiveNode == p){
					CSceneNode * p2 = m_SceneGraph.GetSceneNode((i+1)%nsize);
					m_SceneGraph.setActiveSceneNode(p2);
					updateGL();  // calls draw(), which emits drawFinished()
					break;
				}
			}
			*/
			m_SceneGraph.nextActiveObject();
			updateGL();
			break;

		case Qt::Key_F9:		//Full screen
			toggleFullScreen();
			break;

		case Qt::Key_F12:		//Toggle stereo display
		case Qt::Key_S:
			toggleStereoDisplay();
			break;

		case Qt::Key_Plus:		//Increase io distance in stereo mode
			this->incEyeDistance(+1);
			updateGL();
			break;

		case Qt::Key_Minus:		//Decrease io distance in stereo mode
			this->incEyeDistance(-1);
			updateGL();
			break;

		case Qt::Key_Home:
			{
				Camera *pcam = camera();
				const float s = pcam->flySpeed();
				pcam->setFlySpeed(s*1.5f);
			}
			break;

		case Qt::Key_End:
			{
				Camera *pcam = camera();
				const float s = pcam->flySpeed();
				pcam->setFlySpeed(s/1.5f);
			}
			break;

		default:
			QGLViewer::keyPressEvent(e);
			break;
	}
}



const int BLOCKW = 18;
const int BLOCKH = 24;

static int LEGEND_WIDTH(const int orientation, const int nlabel)
{
	int w;
	if (orientation==0){ //
		w = (BLOCKW+1)*nlabel;
	}
	else{
		w = (BLOCKH+1) + 90;
	}
	return w;
}

static int LEGEND_HEIGHT(const int orientation, const int nlabel)
{
	int h;
	const int texth = 20;
	if (orientation==0){ //
		h = (BLOCKH+1)+ texth;
	}
	else{
		h = (BLOCKW+1) * nlabel;
	}
	return h;
}

void Viewer::_drawVerticalTextureBlocks(
    CGLDrawParms* p, const int sw, const int sh, const int pos,
    unsigned char  rgba[256][4], const int _n)
{
	const int orientation = 1;
	int n=_n, idx;
	int oox = 10;
	int ooy = 10;
	int lw = LEGEND_WIDTH(orientation, n);

	if (n<2) n=2;
    if (n>256) n=256;
	const float kk = 255.0f/n;
	const int bw = BLOCKH;
    const int bh = BLOCKW;
    int ox, oy;

	switch(pos){
	case 0:
		ox = oox; oy = ooy;
		break;
	case 1:
		ox = sw -lw - oox;
		oy = ooy;
		break;
	case 2:
		ox = oox; 
		oy = sh-ooy-LEGEND_HEIGHT(orientation, n);
		break;
	case 3:
		ox = sw -lw - oox;
		oy = sh-ooy-LEGEND_HEIGHT(orientation, n);
		break;
	}

	unsigned char txtclr[3];
	txtclr[0] = p->m_cLegendTextColor.x*255;
	txtclr[1] = p->m_cLegendTextColor.y*255;
	txtclr[2] = p->m_cLegendTextColor.z*255;
	float dx = p->m_fMaxRange - p->m_fMinRange;
	dx = dx / (n-1);

    Vector2i posbuf[256];
	for (int i=0; i<n; i++){
		int posx, posy;
		posx = ox ;
		posy = oy + (bh+1) * (n-1-i);
		idx = (int)(i * kk);
		unsigned char *pcolor = rgba[idx];

		//draw texture block
		glBegin(GL_QUADS);
		glColor3ub(pcolor[0], pcolor[1], pcolor[2]);
		glVertex2d(posx, posy);
		glVertex2d(posx+bw, posy);
		glVertex2d(posx+bw, posy+bh);
		glVertex2d(posx, posy+bh);
		glEnd();

        posbuf[i].x= posx+bw+4;
        posbuf[i].y= posy+0.75f*bh;
	}

    //draw text
    glColor3ub(txtclr[0], txtclr[1], txtclr[2]);
    for (int i=0; i<n; i++){
		char str[100];
		const float tval = p->m_fMinRange + dx * i;
		sprintf(str, "%.2E", (double)tval);
		drawTextGlut(posbuf[i].x, posbuf[i].y, str);
    }
}


void Viewer::_drawHorizonTextureBlocks(CGLDrawParms* p, const int sw, const int sh, const int pos, unsigned char  rgba[256][4], const int n)
{
	const int orientation = 0;
	int nc=n, i, idx, ox, oy, bw, bh;
	int oox = 10;
	int ooy = 10;
	int lw = LEGEND_WIDTH(orientation, n);

	if (nc<2) nc=2;
	const float kk = 255.0f/(nc);

	switch(pos){
	case 0:
		ox = oox; oy = ooy;
		bw = BLOCKW, bh=BLOCKH;
		break;
	case 1:
		ox = sw -lw - oox;
		oy = ooy;
		bw = BLOCKW, bh = BLOCKH;
		break;
	case 2:
		ox = oox; 
		oy = sh-ooy-LEGEND_HEIGHT(orientation, n);
		bw = BLOCKW, bh=BLOCKH;
		break;
	case 3:
		ox = sw -lw - oox;
		oy = sh-ooy-LEGEND_HEIGHT(orientation, n);
		bw = BLOCKW, bh = BLOCKH;
		break;
	}

	unsigned char txtclr[3];
	CGLDrawParms  *pDrawParms = p;		
	txtclr[0] = pDrawParms->m_cLegendTextColor.x*255;
	txtclr[1] = pDrawParms->m_cLegendTextColor.y*255;
	txtclr[2] = pDrawParms->m_cLegendTextColor.z*255;
//	QFont serifFont( "Times", 10 );
	float dx = pDrawParms->m_fMaxRange- pDrawParms->m_fMinRange;
	dx = dx / (n-1);

	for (i=0; i<n; i++){
		int posx, posy;
		posx = ox + (bw+1) * (n-1-i);
		posy = oy;
		idx = (int)(i * kk);
		unsigned char *pcolor = rgba[idx];

		//draw texture block
		glBegin(GL_QUADS);
		glColor3ub(pcolor[0], pcolor[1], pcolor[2]);
		glVertex2d(posx, posy);
		glVertex2d(posx+bw, posy);
		glVertex2d(posx+bw, posy+bh);
		glVertex2d(posx, posy+bh);
		glEnd();

		//draw text
		char str[100];
		float tval = pDrawParms->m_fMinRange+dx*i;
		sprintf(str, "%.2E", (double)tval);
		glColor3ub(txtclr[0], txtclr[1], txtclr[2]);
		drawText(posx, posy+BLOCKH+20, str);
	}
}


void Viewer::_drawTextureBlocks(CGLDrawParms* p, const int pos, const int orientation, unsigned char  rgba[256][4], const int n)
{
	const int sw = width();
	const int sh = height(); 
	int ori = 1;
	if (ori==0){
		_drawHorizonTextureBlocks(p, sw, sh, pos, rgba, n);
	}
	else{
		_drawVerticalTextureBlocks(p, sw, sh, pos, rgba, n);
	}
}


void Viewer::DrawLegend(void)
{
	//invisible text
	//drawText(-100, -100, "ABC");

	const int n = m_SceneGraph.size();
	if (n==0) return;

	CSceneNode *pnode = m_SceneGraph.GetActiveSceneNode();
	if (pnode==NULL)
		return;

	CGLDrawParms  *pDrawParms = &pnode->m_DrawParms;
	if (!pDrawParms->m_bShowLegend)
		return;
	if (!(pDrawParms->m_bEnableTexture1D||pDrawParms->m_bEnableVertexTexture))
		return;

	// Setup the view of the cube. 
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	startScreenCoordinatesSystem();
	int pos = pDrawParms->m_cLegendPosition;
	int orientation = pDrawParms->m_cLegendOrientation;
	int lvl = pDrawParms->m_nLegendLevel;
	_drawTextureBlocks(pDrawParms, pos, orientation, pDrawParms->m_texture1D, lvl);
	stopScreenCoordinatesSystem();
}


void Viewer::fastDraw(void)   	
{
	if (!m_bFastDraw){
		__glDraw();
		return;
	}

	const int BUFFLEN = 2048;
	int i;
	enum CGLDrawParms::glDrawMeshStyle meshbuff[BUFFLEN];
	enum CGLDrawParms::glDrawVertexStyle vertbuff[BUFFLEN];
	int nnode = m_SceneGraph.size();
	if (nnode==0)
		return;
	assert(nnode<BUFFLEN);

	//push drawing state
	for (i=0; i<nnode; i++){
		CSceneNode *p = m_SceneGraph.GetSceneNode(i);
		meshbuff[i]=p->m_DrawParms.m_nDrawType; 
		vertbuff[i]=p->m_DrawParms.m_nDrawVertexStyle; 
		p->m_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_NONE;
		p->m_DrawParms.m_nDrawVertexStyle = CGLDrawParms::DRAW_VERTEX_POINT;
	}
	__glDraw();
	//pop drawing state
	for (i=0; i<nnode; i++){
		CSceneNode *p = m_SceneGraph.GetSceneNode(i);
		p->m_DrawParms.m_nDrawType = meshbuff[i]; 
		p->m_DrawParms.m_nDrawVertexStyle = vertbuff[i]; 
	}
}


void Viewer::__glDraw(void)
{
    ZAccurateTimer tm;
    tm.start();
    rendering_counter ++;

  	if (_pDrawParms){
		setAxisIsDrawn(_pDrawParms->m_bShowAixes);
		//setDisplayZBuffer(!_pDrawParms->m_bEnableDepthTest);
		int r = _pDrawParms->m_cBackgroundColor.x * 255;
		int g = _pDrawParms->m_cBackgroundColor.y * 255;
		int b = _pDrawParms->m_cBackgroundColor.z * 255;
		setBackgroundColor(QColor(r, g, b));
	}

	_showBackgroundImage();

	this->glDrawObject();

	this->DrawLegend();
    glFinish();
    tm.stop();
    const double diff = tm.getTimeInSecond();
    const double fps = 1.0 / (fabs(diff) + 1e-10);
    this->drawOsd(fps);
}


void Viewer::setDrawingData(CGLDrawParms *pDrawStyle)
{
	_pDrawParms = pDrawStyle;
}


void Viewer::glDrawObject(void)
{
	Vec pos=camera()->position();
	Vec dir=camera()->viewDirection();
	_pDrawParms->m_ViewPos = Vector3f(pos.x, pos.y, pos.z);
	_pDrawParms->m_ViewDir = Vector3f(dir.x, dir.y, dir.z);
	_pDrawParms->m_Viewport[0] = 0;
	_pDrawParms->m_Viewport[1] = 0;
	_pDrawParms->m_Viewport[2] = this->width();
	_pDrawParms->m_Viewport[3] = this->height();

	//setup a light at the view position;
	{
		Vector3f lightpos0 = _pDrawParms->m_ViewPos;
		CPointLight light0(lightpos0, 1.20);
		light0.applyLight(0);
	}

	//draw the scene
	m_SceneGraph.glDraw(*_pDrawParms);
	m_SceneGraph.glDrawPickedObject();

	//draw floor;
	m_Floor.glDraw();
}


void Viewer::drawWithNames(void)
{
	m_SceneGraph.glDraw4Picking();
}


void Viewer::postSelection(int x, int y)  	 	
{
	//This method is called at the end of the select() procedure. 
	//It should finalise the selection process and update the data 
	//structure/interface/computation/display... according to the newly selected entity.	
	if (m_SceneGraph.size()==0) return;
	const int objid = selectedName();
	if (objid <0)
		qWarning("No object selected under pixel " + QString::number(x) + "," + QString::number(y));
	else
		fprintf(stderr, "Picked object id is %d\n", objid);

	//call the scene graph method
	m_SceneGraph.postSelection(objid, x, y);
}



QString Viewer::helpString() const
{
  QString text("<h2>View3D</h2>");
  text += "The <code>View3d()</code> function is called instead of <code>draw()</code> when the camera ";
  text += "is manipulated. Providing such a simplified version of <code>draw()</code> allows for interactive ";
  text += "frame rates when the camera is moved, even for very complex scenes.";
  return text;
}


static void exportPosition(CSceneGraph &sceneGraph, const int counter, const int objid, const int vid)
{
	CSceneNode *pnode = sceneGraph.GetSceneNode(0);
	CPolyObj *pobj = pnode->m_pObject;
	Vector3d v = pobj->m_pVertex[0];
	printf("Step%04d V%d %lg %lg %lg\n", counter, vid, v.x, v.y, v.z);
}


void Viewer::_doAnimationLoop(const char *foutname, const char *curext, void *putil)
{
	int i;
	char finname[300], message[400];
	FILE *scenefile=NULL;
	const int OBUFFLENGTH = 100;
	const double scale = 1;
	CCia3dObj * pCia3dObjs[OBUFFLENGTH]; 
	for (i=0; i<OBUFFLENGTH; i++) pCia3dObjs[i]=NULL;

	//write a list of file names for animation;
	if (foutname[0]!=0)
		scenefile = fopen(foutname, "w");

	i=1;
	while (m_bRepeatPlay || i){
		i--;
		int playcount = 0;
		while (isInAnimation()){
			playcount++;
			//load a new mesh;
			getInputFileNames(finname);
			emit printMessage(finname);
			if (!LoadAndSetMultipleObjs(finname, pCia3dObjs, OBUFFLENGTH, scale)){
				sprintf(message, "Unable to load data file: %s", finname); 
				QMessageBox::about( this, "Error message", message);
				i = 0;
				break;
			}
			
			//add to scenegraph
			m_SceneGraph.RemoveAll();
			AxisAlignedBox bbox;
			AddObjectsToSceneGraph(pCia3dObjs, OBUFFLENGTH, m_SceneGraph);

			//use the state var
			CSceneGraphState *pState = (CSceneGraphState *)putil;
			pState->useSceneGraphState(false);
			//call draw(), which emits drawFinished(), which calls saveSnapshot()
			updateGL();  	
			//in case of long operations, process the delayed events
			qApp->processEvents();

			//write the snapshot to an image file
			if (scenefile){
				//get the current counter
				const int counter = get_counter();
				setSnapshotCounter(counter);
				//make a file name
				char sbuff[300]; 
				sprintf(sbuff, "%s%04d.%s", foutname, counter, curext);
				fprintf(scenefile, "%s\n", sbuff);
				if (strcmp(curext, "pov")==0)
					m_SceneGraph.exportPovrayFile(this, sbuff);
				if (strcmp(curext, "txt")==0)
					exportPosition(m_SceneGraph, counter, 0, 0);
				else
					saveSnapshot(true, true);			
			}
			
			//increase the counter
			increaseAnimationCounter();
			if (m_nTimeDelay>0) 
				delayTimeMSecond(m_nTimeDelay);
		}
		if (m_bRepeatPlay)
			resetCurrentAnimationCounter();
		if (playcount==0) break;
	}

	if (scenefile) fclose(scenefile);
}


void Viewer::doAnimation(void)
{
	char *ext[6]={"jpg", "bmp", "png", "ppm", "pov", "txt"}, *curext, foutname[300];

	//set output file name and format;
	this->setSnapshotQuality(99);
	getOutputAnimationFileNames(foutname);
	if (foutname[0]!=0x0){
		setSnapshotFileName(foutname);
		const int slen = strlen(foutname);
		char * const surfix = &foutname[slen-4];
		if (strcmp(surfix, ".jpg")==0 || strcmp(surfix, ".JPG")==0){
			*surfix = 0; curext = ext[0];
			setSnapshotFormat("JPEG");
		}
		else if (strcmp(surfix, ".bmp")==0 || strcmp(surfix, ".BMP")==0){
			*surfix = 0; curext = ext[1];
			setSnapshotFormat("BMP");
		}
		else if (strcmp(surfix, ".png")==0 || strcmp(surfix, ".PNG")==0){
			*surfix = 0; curext = ext[2];
			setSnapshotFormat("PNG");
		}
		else if (strcmp(surfix, ".ppm")==0 || strcmp(surfix, ".PPM")==0){
			*surfix = 0; curext = ext[3];
			setSnapshotFormat("PPM");
		}
		else if (strcmp(surfix, ".pov")==0 || strcmp(surfix, ".POV")==0){
			*surfix = 0; curext = ext[4];
		}
		else if (strcmp(surfix, ".txt")==0 || strcmp(surfix, ".TXT")==0){
			*surfix = 0; curext = ext[5];
		}
		else
			return;
	}


	//save all the old scene nodes
	CSceneGraphState sceneGraphState(m_SceneGraph);
	//animation and close
	_doAnimationLoop(foutname, curext, &sceneGraphState);
	//clear
	sceneGraphState.restoreOldSceneGraph();
	sceneGraphState.clearSceneGraphState();

	//MessageBox::about( this, "Message box", "The animation rendering is completed!");
}


void Viewer::_showBackgroundImage(void)
{
	if (m_pBGImage==NULL)
		return;
	if (m_pBGImage->isNull())
		return;
	int depth = m_pBGImage->depth();
	if (depth!=32) return;

	uchar * pix = m_pBGImage->bits();
	int imagew = m_pBGImage->width();
	int imageh = m_pBGImage->height();
	int winw = this->width();
	int winh = this->height();
	float xf = (float)winw / imagew;
	float yf = (float)winh / imageh;

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	setTextIsEnabled(true);
	startScreenCoordinatesSystem();
	glRasterPos2i(0,winh);
	glPixelZoom(xf, yf);
	glDrawPixels(imagew, imageh, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	stopScreenCoordinatesSystem();
}

void Viewer::resetCurrentAnimationCounter(void)
{
	m_animMutex.lock();
	_nCurrentCount = _nStartCount;
	m_animMutex.unlock();
}

void Viewer::invalidCurrentAnimationCounter(void)
{
	m_animMutex.lock();
	_nCurrentCount = _nEndCount+1;
	m_animMutex.unlock();
}

void Viewer::drawTextGlut(int x, int y, const char *text)
{
	glRasterPos2i(x, y);
	while (*text)
    {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text);
		text++;
	}   
}

void Viewer::drawOsd(const float fps)
{
    const int NUM_LINES = 4;
	const int imageWidth = width();
	const int imageHeight = height();
    const unsigned int totalVert = m_SceneGraph.totalVertexCount();
    const unsigned int totalElm = m_SceneGraph.totalElementCount();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
    this->startScreenCoordinatesSystem();

	int x1 = 4;
	int x2 = 180;
	int w = x2-x1;
	int y1 = imageHeight - 72;
	int lineHeight = 15;
	int margin = 5;
	int y2 = y1 + margin + lineHeight * NUM_LINES;

	glBegin(GL_QUADS);
	{
		glColor3f(.2,.2,.2);
		glVertex2f(x1, y1);
		glVertex2f(x1+(float)w, y1);
		glVertex2f(x1+(float)w, y2);
		glVertex2f(x1, y2);
	}
	glEnd();

	glColor3f(1,1,1);
	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	char buf[256];

	x1 += margin;
	y1 += margin + 10;
    const float fps2 = currentFPS();
	sprintf(buf, "Frame Rate: %.1f, %.1f FPS", fps, fps2);
	drawTextGlut(x1, y1, buf);
	y1 += lineHeight;

    const float tps = totalElm * fps * 1e-6f; 
	sprintf(buf, "Tri Per Sec: %.1f M", tps);
	drawTextGlut(x1, y1, buf);
	y1 += lineHeight;

    sprintf(buf, "Vbo: %d, DispList: %d", (int)m_useVbo, (int)m_useDispList);
	drawTextGlut(x1, y1, buf);
	y1 += lineHeight;

    sprintf(buf, "Vert: %u, Elm: %u", totalVert, totalElm);
	drawTextGlut(x1, y1, buf);
	y1 += lineHeight;

	glPopAttrib();
    this->stopScreenCoordinatesSystem();
}


void Viewer::onDraw(void)
{
    qApp->processEvents();
    this->draw();
}
